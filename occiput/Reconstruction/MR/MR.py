# -*- coding: utf-8 -*-
# occiput
# Harvard University, Martinos Center for Biomedical Imaging
# Aalto University, Department of Computer Science


__all__ = ['KSpace', 'MR_Static_Scan', 'MR_Dynamic_Scan','import_kspace', 'load_motion_sensor_data', 'load_vnav_mprage']

# Import occiput:
from ...Core import Image3D
from ...Visualization.Colors import *
from ...DataSources.Synthetic.Shapes import uniform_cylinder
from ...Visualization.Visualization import ipy_table, has_ipy_table, svgwrite, has_svgwrite
from ...Functional.NiftyRec import has_NiftyPy, INTERPOLATION_POINT, \
    INTERPOLATION_LINEAR
from ...DataSources.FileSources.vNAV import load_vnav_mprage
from ...DataSources.FileSources.MR_motion_sensors import load_motion_sensor_data
from ...DataSources.FileSources.MR_kspace import import_kspace


# Import DisplayNode for IPython integration
#import DisplayNode

# Import other modules
from PIL import ImageDraw
from PIL import Image as PIL
import numpy as np

#from numpy import isscalar, linspace, int32, uint32, ones, zeros, pi,
# float32, where, ndarray, nan, inf, exp, asarray, \
#    complex64, complex128, complex, abs, angle, real, imag
#from numpy.fft import fftn, ifftn, ifft2, fftshift
import scipy as sp
#import scipy.signal
#import scipy.io
#import scipy.ndimage


# Import ilang (inference language; optimisation)
from .MR_ilang import MR_Static_Gaussian, MR_Dynamic_Gaussian, ProbabilisticGraphicalModel

# Set verbose level
from ...global_settings import *

set_verbose_no_printing()
# set_verbose_high()

INTERPOLATION_COMPLEXPLANE = 0
INTERPOLATION_POLAR = 1


class SequenceParameters:
    def __init__(self):
        self.Tr = 0.020
        self.name = "Flash"


class KSpace:
    def __init__(self, filename=None):
        self.parameters = SequenceParameters()
        self.data = None
        self.motion = None
        self.motion_events_indexes = None
        self.filename = None
        if filename is not None:
            self.load_from_file(filename)

    def load_from_file(self, filename, motion_data_file=None):
        self.filename = filename
        mat = sp.io.loadmat(filename)
        self.data = mat["data"]
        # fftshift
        for i in range(self.data.shape[0]):
            self.data[i, :, :, :] = np.fft.fftshift(self.data[i, :, :, :])
        # self.data = self.data.squeeze()
        if motion_data_file is not None:
            self.load_motion_from_file(motion_data_file)

    def load_motion_from_file(self, filename):
        motion = load_motion_sensor_data(filename)
        self.motion = motion  # FIXME: define motion information
        motion_events = self.motion.extract_motion_events()
        motion_events_indexes = np.where(motion_events != 0)[0].tolist()
        self.motion_events_indexes = motion_events_indexes

    def get_static_data(self):
        data = np.complex64(
            np.zeros((self.data.shape[1], self.data.shape[2], self.data.shape[3]))
        )
        for i in range(self.data.shape[0]):
            data = data + self.data[i, :, :, :]
        return data


class MR_Static_Scan:
    def __init__(self):
        self.kspace = None

    def load_kspace(self, filename):
        self.kspace = KSpace()
        self.kspace.load_from_file(filename)
        self._construct_ilang_model()

    def reconstruct_iterative(
        self, method=None, iterations=20, tol=None, dt=0.4, beta=0.05
    ):
        kspace = self.kspace.get_static_data()
        shape = kspace.shape
        image = np.asarray(np.complex64(np.zeros(shape)), order="F")
        for iter in range(iterations):
            diff = kspace - self.project(image)
            grad = -self.backproject(diff)
            image_abs = abs(image)
            kernel = -np.ones((3, 3, 3))
            kernel[1, 1, 1] = 26
            # 1) smooth magnitude
            eps = 1e-8
            norm = (
                2
                * (sp.ndimage.filters.convolve(image_abs, kernel) + eps)
                / (image_abs + eps)
            )
            # !!!
            # 2) smooth square magnitude
            # norm = 4*(sp.ndimage.filters.convolve(image_abs,kernel))
            g0_real = -np.real(image) * norm
            g0_imag = -np.imag(image) * norm
            g0 = np.complex64(g0_real + g0_imag * 1j)
            image = image - dt * (grad - beta * g0)
        return Image3D(abs(image))

    def reconstruct_ifft(self):
        kspace = self.kspace.get_static_data()
        image = np.asarray(np.fft.fftshift(np.fft.ifftn(kspace)), order="F")
        return Image3D(abs(image))

    def project(self, volume):
        return np.asarray(np.fft.fftn(np.fft.fftshift(volume)), order="F")

    def backproject(self, kspace):
        return np.asarray(np.fft.fftshift(np.fft.ifftn(kspace)), order="F")

    def _construct_ilang_model(self):
        # define the ilang probabilistic model
        self.ilang_model = MR_Static_Gaussian(self)
        # construct a basic Directed Acyclical Graph
        self.ilang_graph = ProbabilisticGraphicalModel(["x", "k", "sigma"])
        self.ilang_graph.set_nodes_given(["k", "sigma"], True)
        self.ilang_graph.add_dependence(
            self.ilang_model, {"x": "x", "k": "k", "sigma": "sigma"}
        )
        # construct a basic sampler object
        # self.sampler     = Sampler(self.ilang_graph)

    def _repr_html_(self):
        if not has_ipy_table:
            return "Please install ipy_table."
        table_data = [["bla   ", 1], ["blabla", 2]]

        table = ipy_table.make_table(table_data)
        table = ipy_table.apply_theme("basic_left")
        table = ipy_table.set_global_style(float_format="%3.3f")
        return table._repr_html_()


class MR_Dynamic_Scan:
    def __init__(self):
        self.kspace = None
        self.time_bins = None

    def load_kspace(self, filename, motion_data_file=None):
        self.kspace = KSpace()
        self.kspace.load_from_file(filename, motion_data_file)
        N_time_bins = len(self.kspace.motion_events_indexes) + 1
        self.time_bins = [0] + self.kspace.motion_events_indexes
        self._construct_ilang_model()

    def reconstruct_iterative(
        self,
        method=None,
        iterations=20,
        tol=None,
        dt=0.9,
        beta=0.05,
        active_frames=None,
        interpolation_space=INTERPOLATION_COMPLEXPLANE,
        interpolation_mode=INTERPOLATION_POINT,
    ):
        shape = self.kspace.get_static_data().shape
        image = np.asarray(np.complex128(np.zeros(shape)), order="F")
        n_frames = len(self.time_bins)
        for iter in range(iterations):
            grad = np.asarray(np.complex128(np.zeros(shape)), order="F")
            for framei in range(n_frames):
                use_frame = True
                if active_frames is not None:
                    if framei in active_frames:
                        use_frame = True
                    else:
                        use_frame = False
                if use_frame:
                    print(("time frame: %d / %d " % (framei, n_frames)))
                    if framei == 0:
                        image_tr = image
                    else:
                        M = self._affine
                        image_tr = self._transform_image(
                            image,
                            M,
                            interpolation_space=interpolation_space,
                            interpolation_mode=interpolation_mode,
                        )
                    kspace = np.asarray(self.kspace.data[framei, :, :, :], order="F")
                    diff = kspace - self.project(image_tr)
                    grad_tr = -self.backproject(diff)
                    if framei == 0:
                        grad += grad_tr
                    else:
                        grad += self._transform_image(
                            grad_tr,
                            M.inverse,
                            interpolation_space=interpolation_space,
                            interpolation_mode=interpolation_mode,
                        )
            image_abs = abs(image)
            kernel = -np.ones((3, 3, 3))
            kernel[1, 1, 1] = 26
            # 1) smooth magnitude
            eps = 1e-8
            norm = (
                2
                * (sp.ndimage.filters.convolve(image_abs, kernel) + eps)
                / (image_abs + eps)
            )
            # 2) smooth square magnitude
            # norm = 4*(sp.ndimage.filters.convolve(image_abs,kernel))
            g0_real = np.real(image) * norm
            g0_imag = np.imag(image) * norm
            g0 = g0_real + g0_imag * 1j
            image = image - dt * (grad + beta * g0)
        return Image3D(abs(image))

    def _set_affine(self, affine):
        self._affine = affine

    def _transform_image(
        self,
        image,
        affine,
        interpolation_space=INTERPOLATION_COMPLEXPLANE,
        interpolation_mode=INTERPOLATION_POINT,
    ):
        I = Image3D(np.zeros(image.shape))
        I.set_space("world")
        grid = I.get_world_grid()
        I.transform(affine)
        if interpolation_space == INTERPOLATION_COMPLEXPLANE:
            re = np.asarray(np.real(image), order="F")
            im = np.asarray(np.imag(image), order="F")
            I.data = re
            re = I.compute_resample_on_grid(
                grid, interpolation_mode=interpolation_mode
            ).data
            I.data = im
            im = I.compute_resample_on_grid(
                grid, interpolation_mode=interpolation_mode
            ).data
            image_tr = np.complex128(re + im * 1j)
        elif interpolation_space == INTERPOLATION_POLAR:
            r = np.asarray(np.abs(image), order="F")
            t = np.asarray(np.angle(image), order="F")
            I.data = r
            r = I.compute_resample_on_grid(
                grid, interpolation_mode=interpolation_mode
            ).data
            I.data = t
            t = I.compute_resample_on_grid(
                grid, interpolation_mode=interpolation_mode
            ).data
            image_tr = r * np.exp(t * 1j)
        return image_tr

    def reconstruct_ifft(self):
        kspace = self.kspace.get_static_data()
        image = np.asarray(np.fft.fftshift(np.fft.ifftn(kspace)), order="F")
        return Image3D(abs(image))

    def project(self, volume):
        return np.asarray(np.fft.fftn(np.fft.fftshift(volume)), order="F")

    def backproject(self, kspace):
        return np.asarray(np.fft.fftshift(np.fft.ifftn(kspace)), order="F")

    def _construct_ilang_model(self):
        # define the ilang probabilistic model
        self.ilang_model = MR_Dynamic_Gaussian(self)

    def _repr_html_(self):
        if not has_ipy_table:
            return "Please install ipy_table."
        table_data = [["bla   ", 1], ["blabla", 2]]

        table = ipy_table.make_table(table_data)
        table = ipy_table.apply_theme("basic_left")
        table = ipy_table.set_global_style(float_format="%3.3f")
        return table._repr_html_()

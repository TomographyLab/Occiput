# occiput
# Michele Scipioni
# University of Pisa, Italy
# Jan 2018, Pisa

import warnings as __warnings
import nibabel as __nibabel
import occiput as __occiput

with __warnings.catch_warnings():
    __warnings.simplefilter("ignore")
    from nipy.io.nifti_ref import nifti2nipy as nifti_to_nipy


def nibabel_to_occiput(nib):
    """Convert nipy image to Occiput ImageND image.

        Args:
            img (nipy): nipy image.

        Returns:
            ImageND: occiput image. """
    ndim = len(nib.shape)
    if ndim == 3:
        im = __occiput.Core.Core.Image3D(
            data=nib.get_data(), affine=nib.affine,
            space="world", header=nib.header)
    else:
        im = __occiput.Core.Core.ImageND(
            data=nib.get_data(), affine=nib.affine,
            space="world", header=nib.header)
    return im


def nifti_to_occiput(nif):
    """Convert Nifti image to occiput ImageND image.

        Args:
            nif (Nifti): Nifti image.

        Returns:
            ImageND: occiput image.
        """
    nip = nifti_to_nipy(nif)
    return nibabel_to_occiput(nip)


def occiput_to_nifti(occ):
    """Conver occiput ImageND to Nifti image.

        Args:
            occ (ImageND): occiput ImageND image.

        Returns:
            Nifti: Nifti image.
        """
    nii = __nibabel.nifti1.Nifti1Image(occ.data, occ.affine.data, occ.header)
    return nii


def occiput_from_array(array):
    """Numpy ndarray to occiput ImageND image.

        Args:
            array (ndarray): numpy.ndarray.

        Returns:
            ImageND: occiput ImageND image.
        """
    if array.ndim == 3:
        im = __occiput.Core.Core.Image3D(
            data=array, space="world")
    else:
        raise ("Currently only conversion of 3D arrays is supported. ")
    return im

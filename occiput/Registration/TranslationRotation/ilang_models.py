# -*- coding: utf-8 -*-
# occiput
# Harvard University, Martinos Center for Biomedical Imaging
# Aalto University, Department of Computer Science

import numpy as np
from occiput.Core import Image3D, Grid3D, Transform_6DOF
from ...Functional.ilang.Models import Model


class SSD_ilang(Model):
    variables = {
        "source": "continuous",
        "target": "continuous",
        "transformation": "continuous",
        "sigma": "continuous",
    }
    dependencies = [
        ["source", "target", "directed"],
        ["transformation", "target", "directed"],
        ["sigma", "target", "directed"],
    ]

    def __init__(self, name=None):
        if name == None:
            name = self.__class__.__name__
        Model.__init__(self, name)

    def log_conditional_probability_transformation(self, transformation):
        source = self.get_value("source")
        target = self.get_value("target")
        sigma = self.get_value("sigma")
        log_p = 0.0
        return log_p

    def log_conditional_probability_gradient_transformation(self, transformation):
        source = self.get_value("source")
        target = self.get_value("target")
        sigma = self.get_value("sigma")
        gradient = np.asarray([0.0, 0.0, 0.0, 0.0, 0.0, 0.0])
        return gradient

    def sample_conditional_probability_target(self):
        return 0

    def init(self):
        pass

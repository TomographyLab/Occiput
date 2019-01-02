# -*- coding: utf-8 -*-
# occiput
# Harvard University, Martinos Center for Biomedical Imaging
# Aalto University, Department of Computer Science

__all__ = ['MR','PET','SPECT']

from . import MR
from . import PET
from . import SPECT

from .PET import *
from .SPECT import *
from .MR import *

__all__.extend(PET.__all__)
__all__.extend(SPECT.__all__)
__all__.extend(MR.__all__)
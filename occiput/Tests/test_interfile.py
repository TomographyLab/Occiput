# interfile - Interfile read and write
# Michele Scipioni
# University of Pisa, Italy
# Nov 2018, Pisa, Italy

import unittest
from ..Core import Interfile
import pickle


class TestInterfile(unittest.TestCase): 
    """Sequence of tests for module interfile. """ 
    def setUp(self):
        pass

    def test_listmode_parse(self):
        """Parse a simple interfile. """
        with open('../Data/interfile_examples/parsed_listmode.pickle',
                  'rb') as handle:
            listmode_ref = pickle.load(handle)
        listmode = Interfile.load('../Data/interfile_xamples/pet_listmode.l.hdr')
        self.assertEqual(listmode_ref, listmode)

    def test_sinogram_parse(self):
        """Parse a simple interfile. """
        with open('../Data/interfile_examples/parsed_sinogram.pickle', 'rb') as handle:
            sino_ref = pickle.load(handle)
        sino = Interfile.load('../Data/interfile_examples/pet_sinogram.s.hdr')
        self.assertEqual(sino_ref, sino)


if __name__=="__main__": 
    unittest.main() 
import unittest
from mantid.simpleapi import *
import SANSUtility as su

class TestSliceStringParser(unittest.TestCase):
    
    def checkValues(self, list1, list2):

        def _check_single_values( v1, v2):
            self.assertAlmostEqual(v1, v2)

        self.assertEqual(len(list1), len(list2))
        for v1,v2 in zip(list1, list2):
            start_1,stop_1 = v1
            start_2, stop_2 = v2
            _check_single_values(start_1, start_2)
            _check_single_values(stop_1, stop_2)

    def test_checkValues(self):
        """sanity check to ensure that the others will work correctly"""
        values = [
            [[1,2],],
            [[None, 3],[4, None]],
        ]
        for singlevalues in values:
            self.checkValues(singlevalues, singlevalues)
    
    def test_parse_strings(self):
        inputs = { '1-2':[[1,2]],         # single period syntax  min < x < max
                   '1.3-5.6':[[1.3,5.6]], # float
                   '1-2,3-4':[[1,2],[3,4]],# more than one slice
                   '>1':[[1, -1]],       # just lower bound
                   '<5':[[-1, 5]],      # just upper bound
                   '<5,8-9': [[-1, 5], [8,9]],
                   '1:2:5': [[1,3], [3,5]] # sintax: start, step, stop                   
            }

        for (k, v) in inputs.items(): 
            self.checkValues(su.sliceParser(k),v)

    def test_accept_spaces(self):
        self.checkValues(su.sliceParser("1 - 2, 3 - 4"), [[1,2],[3,4]])
        
    def test_invalid_values_raise(self):
        invalid_strs = ["5>6", ":3:", "MAX<min"]
        for val in invalid_strs:
            self.assertRaises(SyntaxError, su.sliceParser, val)

    def test_empty_string_is_valid(self):
        self.checkValues(su.sliceParser(""), [[-1,-1]])

    def test_extract_spectra(self):
        mtd.clear()

        ws = CreateSampleWorkspace("Histogram", "Multiple Peaks")
        det_ids = [100, 102, 104]

        result = su.extract_spectra(ws, det_ids, "result")

        # Essentially, do we end up with our original workspace and the resulting
        # workspace in the ADS, and NOTHING else?
        self.assertTrue("result" in mtd)
        self.assertTrue("ws" in mtd)
        self.assertEquals(2, len(mtd))

        self.assertEquals(result.getNumberHistograms(), len(det_ids))
        self.assertEquals(result.getDetector(0).getID(), 100)
        self.assertEquals(result.getDetector(1).getID(), 102)
        self.assertEquals(result.getDetector(2).getID(), 104)

if __name__ == "__main__":
    unittest.main()

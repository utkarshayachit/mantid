import unittest
from mantid.simpleapi import *
import SANSUtility as su

class SANSUtilityTest(unittest.TestCase):
    
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

        ws = CreateSampleWorkspace("Histogram", "Multiple Peaks")
        det_ids = range(100, 299, 2)
        result = su.extract_spectra(ws, det_ids, "result")

    def test_yield_ws_indices_from_det_ids(self):
        ws = CreateSampleWorkspace("Histogram", "Multiple Peaks")

        ws_indices = list(su.yield_ws_indices_from_det_ids(ws, [100, 102, 104]))

        self.assertTrue(0 in ws_indices)
        self.assertTrue(2 in ws_indices)
        self.assertTrue(4 in ws_indices)
        self.assertEquals(len(ws_indices), 3)

    def test_get_masked_det_ids(self):
        ws = CreateSampleWorkspace("Histogram", "Multiple Peaks")

        MaskDetectors(Workspace=ws, DetectorList=[100, 102, 104])

        masked_det_ids = list(su.get_masked_det_ids(ws))

        self.assertTrue(100 in masked_det_ids)
        self.assertTrue(102 in masked_det_ids)
        self.assertTrue(104 in masked_det_ids)
        self.assertEquals(len(masked_det_ids), 3)

    def test_get_det_ids_in_component(self):
        ws = CreateSampleWorkspace("Histogram", "Multiple Peaks")
        bank1_det_ids = su.get_det_ids_in_component(ws, "bank1")
        bank2_det_ids = su.get_det_ids_in_component(ws, "bank2")

        self.assertEquals(len(bank1_det_ids), 100)
        self.assertEquals(len(bank2_det_ids), 100)

        self.assertTrue(100 in bank1_det_ids)
        self.assertTrue(200 in bank2_det_ids)

    def test_merge_to_ranges(self):
        self.assertEquals([[1, 4]],                 su._merge_to_ranges([1, 2, 3, 4]))
        self.assertEquals([[1, 3], [5, 7]],         su._merge_to_ranges([1, 2, 3, 5, 6, 7]))
        self.assertEquals([[1, 3], [5, 5], [7, 9]], su._merge_to_ranges([1, 2, 3, 5, 7, 8, 9]))
        self.assertEquals([[1, 1]],                 su._merge_to_ranges([1]))

if __name__ == "__main__":
    unittest.main()

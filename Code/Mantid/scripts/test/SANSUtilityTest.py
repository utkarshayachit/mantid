import unittest
from mantid.simpleapi import *
import SANSUtility as su

class SANSUtilityTest(unittest.TestCase):

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

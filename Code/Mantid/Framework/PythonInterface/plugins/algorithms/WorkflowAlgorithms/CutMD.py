from mantid.kernel import *
from mantid.api import *
from mantid.simpleapi import *
import numpy as np
import os.path
import re

class Projection(object):
    u = "u"
    v = "v"
    w = "w"
    
class ProjectionUnit(object):
    r = "r"
    a = "a"
    
    
class CutMD(DataProcessorAlgorithm):
    
    def category(self):
        return 'MDAlgorithms'

    def summary(self):
        return 'Slices multidimensional workspaces using input projection information and binning limits.'

    def PyInit(self):
        self.declareProperty(IMDEventWorkspaceProperty('InputWorkspace', '', direction=Direction.Input),
                             doc='MDWorkspace to slice')
        
        self.declareProperty(ITableWorkspaceProperty('Projection', '', direction=Direction.Input, optional = PropertyMode.Optional), doc='Projection')
        
        self.declareProperty(FloatArrayProperty(name='P1Bin', values=[]),
                             doc='Projection 1 binning')
        
        self.declareProperty(FloatArrayProperty(name='P2Bin', values=[]),
                             doc='Projection 2 binning')
        
        self.declareProperty(FloatArrayProperty(name='P3Bin', values=[]),
                             doc='Projection 3 binning')
        
        self.declareProperty(FloatArrayProperty(name='P4Bin', values=[]),
                             doc='Projection 4 binning')

        self.declareProperty(IMDWorkspaceProperty('OutputWorkspace', '',
                             direction=Direction.Output),
                             doc='Output cut workspace')
        
        self.declareProperty(name="NoPix", defaultValue=False, doc="If False creates a full MDEventWorkspaces as output. True to create an MDHistoWorkspace as output. This is DND only in Horace terminology.")
        
        self.declareProperty(name="CheckAxes", defaultValue=True, doc="Check that the axis look to be correct, and abort if not.")
        
    
    def __calculate_steps(self, extents, horace_binning ):
        # Because the step calculations may involve moving the extents, we re-publish the extents.
        out_extents = extents
        out_n_bins = list()
        for i in range(len(horace_binning)):

            n_arguments = len(horace_binning[i])
            max_extent_index = (i*2) + 1
            min_extent_index = (i*2)
            dim_range = extents[ max_extent_index ] - extents[ min_extent_index ]
            
            if n_arguments == 0:
                raise ValueError("binning parameter cannot be empty")
            elif n_arguments == 1:
                step_size = horace_binning[i][0]
                if step_size > dim_range:
                     step_size = dim_range
                n_bins = int( dim_range / step_size)
                # Correct the max extent to fit n * step_size
                out_extents[max_extent_index] = extents[min_extent_index] + ( n_bins * step_size )
            elif n_arguments == 2:
                out_extents[ min_extent_index ] = horace_binning[i][0]
                out_extents[ max_extent_index ] = horace_binning[i][1]
                n_bins = 1
            elif n_arguments == 3:
                dim_min = horace_binning[i][0]
                dim_max = horace_binning[i][2]
                step_size = horace_binning[i][1]
                dim_range = dim_max - dim_min
                if step_size > dim_range:
                     step_size = dim_range
                n_bins = int( dim_range / step_size)
                # Correct the max extent to fit n * step_size
                out_extents[ max_extent_index ] = dim_min  + ( n_bins * step_size )
                out_extents[ min_extent_index ] = dim_min
            if n_bins < 1:
                raise ValueError("Number of bins calculated to be < 1")
            out_n_bins.append( n_bins )
        return out_extents, out_n_bins
        
    def __extents_in_current_projection(self, to_cut, dimension_index):
        
        dim = to_cut.getDimension(dimension_index)
        dim_min = dim.getMinimum()
        dim_max = dim.getMaximum()
        return (dim_min, dim_max)
      
    def __calculate_extents(self, v, u, w, limits):
        M=np.array([u,v,w])
        Minv=np.linalg.inv(M)
    
        # unpack limits
        Hrange, Krange, Lrange = limits
    
        # Create a numpy 2D array. Makes finding minimums and maximums for each transformed coordinates over every corner easier.
        new_coords = np.empty([8, 3])
        counter = 0
        for h in Hrange:
           for k in Krange:
               for l in Lrange:
                   original_corner=np.array([h,k,l])
                   new_coords[counter]=original_corner.dot(Minv)
                   counter += 1
        
        # Get the min max extents
        extents = list()
        for i in range(0,3):
            # Vertical slice down each corner for each dimension, then determine the max, min and use as extents
            extents.append(np.amin(new_coords[:,i]))
            extents.append(np.amax(new_coords[:,i]))
        
        return extents
        
    def __uvw_from_projection_table(self, projection_table):
        if not isinstance(projection_table, ITableWorkspace):
            I = np.identity(3)
            return (I[0, :], I[1, :], I[2, :])
        column_names = projection_table.getColumnNames()
        u = np.array(projection_table.column(Projection.u))
        v = np.array(projection_table.column(Projection.v))
        if not Projection.w in column_names:
            w = np.cross(v,u)
        else:
            w = np.array(projection_table.column(Projection.w))
        return (u, v, w)
    
    def __units_from_projection_table(self, projection_table):
        if not isinstance(projection_table, ITableWorkspace) or not "type" in projection_table.getColumnNames():
            units = (ProjectionUnit.r, ProjectionUnit.r, ProjectionUnit.r)
        else:
            units = tuple(projection_table.column("type"))
        return units
            
    
    def __make_labels(self, projection):

        class Mapping:
            
            def __init__(self, replace):
                self.__replace = replace
                
            def replace(self, entry):
                if np.absolute(entry) == 1:
                    if entry > 0:
                        return self.__replace 
                    else:
                        return "-" + self.__replace
                elif entry == 0:
                    return 0
                else:
                    return "%.2f%s" % ( entry, self.__replace )
                return entry
                
        crystallographic_names = ['zeta', 'eta', 'xi' ]  
        labels = list()
        for i in range(len(projection)):
            cmapping = Mapping(crystallographic_names[i])
            labels.append( [cmapping.replace(x) for x in projection[i]  ] )
        
        return labels
    
    
    def __verify_input_workspace(self, to_cut):
        coord_system = to_cut.getSpecialCoordinateSystem()
        if not coord_system == SpecialCoordinateSystem.HKL:
            raise ValueError("Input Workspace must be in reciprocal lattice dimensions (HKL)")
        
        ndims = to_cut.getNumDims()
        if ndims < 3 or ndims > 4:
            raise ValueError("Input Workspace should be 3 or 4 dimensional")
        
        # Try to sanity check the order of the dimensions. This is important.
        axes_check = self.getProperty("CheckAxes").value
        if axes_check:
            predicates = ["^(H.*)|(\\[H,0,0\\].*)$","^(K.*)|(\\[0,K,0\\].*)$","^(L.*)|(\\[0,0,L\\].*)$"]  
            for i in range(ndims):
                dimension = to_cut.getDimension(i)
                p = re.compile(predicates[i])
                if not p.match( dimension.getName() ):
                    raise ValueError("Dimensions must be in order H, K, L")

    def __verify_projection_input(self, projection_table):
        if isinstance(projection_table, ITableWorkspace):
            column_names = set(projection_table.getColumnNames())
            logger.warning(str(column_names)) 
            if not column_names == set([Projection.u, Projection.v, 'type']):
                    if not column_names == set([Projection.u, Projection.v, 'offsets', 'type']):
                        if not column_names == set([Projection.u, Projection.v, Projection.w, 'offsets', 'type']):
                            raise ValueError("Projection table schema is wrong! Column names received: " + str(column_names) )
            if projection_table.rowCount() != 3:
                raise ValueError("Projection table expects 3 rows")
            
            
    def __scale_projection(self, (u, v, w), origin_units, target_units, to_cut):
        
        if set(origin_units) == set(target_units):
            return (u,v,w) # Nothing to do.
        
        ol = to_cut.getExperimentInfo(0).sample().getOrientedLattice()
        
        projection_scaled = [u, v, w]
        
        to_from_pairs = zip(origin_units, target_units)
        for i in range(len(to_from_pairs)) :
            
            proj = projection_scaled[i]
            d_star =  2 * np.pi * ol.dstar( float(proj[0]), float(proj[1]), float(proj[2]) )
            
            from_unit, to_unit = to_from_pairs[i]
            if from_unit == to_unit:
                continue
            elif from_unit == ProjectionUnit.a: # From inverse Angstroms to rlu
                projection_scaled[i] *= d_star
            else: # From rlu to inverse Anstroms
                projection_scaled[i] /= d_star
        return projection_scaled
            
        
    def PyExec(self):
        to_cut = self.getProperty("InputWorkspace").value
        self.__verify_input_workspace(to_cut)
        ndims = to_cut.getNumDims()
        
        nopix = self.getProperty("NoPix").value
        
        projection_table = self.getProperty("Projection").value
        self.__verify_projection_input(projection_table)
            
        p1_bins = self.getProperty("P1Bin").value
        p2_bins = self.getProperty("P2Bin").value
        p3_bins = self.getProperty("P3Bin").value
        p4_bins = self.getProperty("P4Bin").value 
        
        x_extents = self.__extents_in_current_projection(to_cut, 0);
        y_extents = self.__extents_in_current_projection(to_cut, 1);
        z_extents = self.__extents_in_current_projection(to_cut, 2); 
        
        projection = self.__uvw_from_projection_table(projection_table)
        target_units = self.__units_from_projection_table(projection_table)
        origin_units = (ProjectionUnit.r, ProjectionUnit.r, ProjectionUnit.r) # TODO. This is a hack!
    
        u,v,w = self.__scale_projection(projection, origin_units, target_units, to_cut)
   
        extents = self.__calculate_extents(v, u, w, ( x_extents, y_extents, z_extents ) )
        extents, bins = self.__calculate_steps( extents, ( p1_bins, p2_bins, p3_bins ) )
        if p4_bins:
            if (ndims == 4):
                ebins = self.__to_mantid_slicing_binning(p1_bins, to_cut, 3); 
                e_units = to_cut.getDimension(3).getUnits()
                bins.append(int(ebins[2]))
                target_units.append(e_units)
            else:
                raise ValueError("Cannot specify P4Bins unless the workspace is of sufficient dimensions")
        
        projection_labels = self.__make_labels(projection)
        
        '''
        Actually perform the binning operation
        '''
        cut_alg_name = "BinMD" if nopix else "SliceMD"
        cut_alg = AlgorithmManager.Instance().create(cut_alg_name)
        cut_alg.setChild(True)
        cut_alg.initialize()
        cut_alg.setProperty("InputWorkspace", to_cut)
        cut_alg.setPropertyValue("OutputWorkspace", "sliced")
        cut_alg.setProperty("NormalizeBasisVectors", False)
        cut_alg.setProperty("AxisAligned", False)
        # Now for the basis vectors.
        for i in range(0, to_cut.getNumDims()):
            if i <= 2:
                label = projection_labels[i]
                unit = target_units[i]
                vec = projection[i]
                value = "%s, %s, %s" % ( label, unit, ",".join(map(str, vec))) 
                cut_alg.setPropertyValue("BasisVector{0}".format(i) , value)
            if i > 2:
                raise RuntimeError("Not implemented yet for non-crystallographic basis vector generation.")
        cut_alg.setProperty("OutputExtents", extents)
        cut_alg.setProperty("OutputBins", bins)
         
        cut_alg.execute()
        
        slice = cut_alg.getProperty("OutputWorkspace").value
        
         # Attach the w-matrix (projection matrix)
        if slice.getNumExperimentInfo() > 0:
            u, v, w = projection
            w_matrix = np.array([u, v, w]).flatten().tolist()
            info = slice.getExperimentInfo(0)
            info.run().addProperty("W_MATRIX", w_matrix, True)
            
        self.setProperty("OutputWorkspace", slice)
        
AlgorithmFactory.subscribe(CutMD)

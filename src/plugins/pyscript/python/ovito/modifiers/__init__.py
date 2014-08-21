"""
This module contains the modifiers of OVITO.

The abstract base class of all modifier types is the :py:class:`Modifier` class.
Typically you create a modifier instance, set its parameters, and finally insert it into a
modification pipeline, e.g.::

    m = AssignColorModifier()
    m.color = (0.2, 1.0, 0.9)
    node.modifiers.append(m)
    
The following modifier types are available:

============================================== =========================================
Class name                                     User interface name
============================================== =========================================
:py:class:`AffineTransformationModifier`       :guilabel:`Affine transformation`
:py:class:`AmbientOcclusionModifier`           :guilabel:`Ambient occlusion`
:py:class:`AssignColorModifier`                :guilabel:`Assign color`
:py:class:`AtomicStrainModifier`               :guilabel:`Atomic strain`
:py:class:`BinAndReduceModifier`               :guilabel:`Bin and reduce`
:py:class:`BondAngleAnalysisModifier`          :guilabel:`Bond-angle analysis`
:py:class:`CalculateDisplacementsModifier`     :guilabel:`Displacement vectors`
:py:class:`CentroSymmetryModifier`             :guilabel:`Centrosymmetry parameter`
:py:class:`ClearSelectionModifier`             :guilabel:`Clear selection`
:py:class:`ClusterAnalysisModifier`            :guilabel:`Cluster analysis`
:py:class:`ColorCodingModifier`                :guilabel:`Color coding`
:py:class:`CommonNeighborAnalysisModifier`     :guilabel:`Common neighbor analysis`
:py:class:`CoordinationNumberModifier`         :guilabel:`Coordination analysis`
:py:class:`CreateBondsModifier`                :guilabel:`Create bonds`
:py:class:`CreateExpressionPropertyModifier`   :guilabel:`Compute property`
:py:class:`DeleteParticlesModifier`            :guilabel:`Delete selected particles`
:py:class:`FreezePropertyModifier`             :guilabel:`Freeze property`
:py:class:`HistogramModifier`                  :guilabel:`Histogram`
:py:class:`InvertSelectionModifier`            :guilabel:`Invert selection`
:py:class:`ManualSelectionModifier`            :guilabel:`Manual selection`
:py:class:`ScatterPlotModifier`                :guilabel:`Scatter plot`
:py:class:`SelectExpressionModifier`           :guilabel:`Expression select`
:py:class:`SelectParticleTypeModifier`         :guilabel:`Select particle type`
:py:class:`ShowPeriodicImagesModifier`         :guilabel:`Show periodic images`
:py:class:`SliceModifier`                      :guilabel:`Slice`
:py:class:`SpatialCorrelationFunctionModifier` :guilabel:`Spatial correlation function`
:py:class:`WignerSeitzAnalysisModifier`        :guilabel:`Wigner-Seitz defect analysis`
:py:class:`WrapPeriodicImagesModifier`         :guilabel:`Wrap at periodic boundaries`
============================================== =========================================
"""

# Load the native module.
from PyScriptScene import Modifier

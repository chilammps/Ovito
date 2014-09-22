==================================
Examples
==================================

This page will provide a collection of example scripts. So far there is only one example.

----------------------------------
Computing Voronoi indices
----------------------------------

This script demonstrates the use of the Voronoi analysis modifier.
The script calculates the distribution of Voronoi coordination polyhedra in an amorphous structure.

A Voronoi polyhedron is expressed in terms of the Schlaefli notation,
which is a vector of indices (n\ :sub:`1`, n\ :sub:`2`, n\ :sub:`3`, n\ :sub:`4`, n\ :sub:`5`, n\ :sub:`6`, ...),
where n\ :sub:`i` is the number of polyhedron faces with *i* edges/vertices.

The script computes the distribution of these Voronoi index vectors
and lists the 10 most frequent polyhedron types in the simulation. In the case
of a Cu\ :sub:`64%`-Zr\ :sub:`36%` bulk metallic glass, the most frequent polyhedron type is the icosahedron.
It has 12 faces with five edges each. Thus, the corresponding Voronoi index 
vector is:

   (0, 0, 0, 0, 12, 0, ...)
   
Python script:
   
.. literalinclude:: ../../../examples/scripts/voronoi_analysis.py
   :lines: 19-

Program output:

.. code-block:: none

  (0, 0, 0, 0, 12, 0)     12274   (11.4 %)
  (0, 0, 0, 2, 8, 2)      7485    (6.9 %)
  (0, 0, 0, 3, 6, 4)      5637    (5.2 %)
  (0, 0, 0, 1, 10, 2)     4857    (4.5 %)
  (0, 0, 0, 3, 6, 3)      3415    (3.2 %)
  (0, 0, 0, 2, 8, 1)      2927    (2.7 %)
  (0, 0, 0, 1, 10, 5)     2900    (2.7 %)
  (0, 0, 0, 1, 10, 4)     2068    (1.9 %)
  (0, 0, 0, 2, 8, 6)      2063    (1.9 %)
  (0, 0, 0, 2, 8, 5)      1662    (1.5 %)
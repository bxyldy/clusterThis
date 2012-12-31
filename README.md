The clusterThis mantra geometry instancing procedural DSO/OTL will instance geometry to
incoming points and will process the point attributes in a number of ways for the instanced geometry.
It delays geomtery instantiation until render time thus, reduces size of IFD file and network bandwidth,
allows access to render time data, add additional detail to scene, post process the instanced geometry, etc.

Features:

* Instance points or primitives (sphere, cube, grid, tube, circle, curve, metaball) at render time

* CVEX processing of instanced geometry

* Open VDB post processing of instanced geometry and it's attributes

* Motion blur (velocity or geometry) can use the "backtrack" or the "v" attribute

* Instance disk file (and apply CVEX deformation and attribute processing)

* Cache file stores data when generating deep shadow maps to optimize instancing.






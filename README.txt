/*****************************************************************************
* $RCSfile: README.txt,v $
*
* Description : This DSO  will instance geomtery or volumes during the render of mantra IFD's
*
* $Revision: 1.13 $
*
* $Source: /dca/cvsroot/houdini/VRAY_clusterThis/README.txt,v $
*
* $Author: mstory $
*
*    Digital Cinema Arts (C) 2008
*
* This work is licensed under the Creative Commons Attribution-ShareAlike 2.5 License.
* To view a copy of this license, visit http://creativecommons.org/licenses/by-sa/2.5/ or send a letter to
* Creative Commons, 543 Howard Street, 5th Floor, San Francisco, California, 94105, USA.
*
***************************************************************************** */

++++++++++++++++++++++++++++++++++++++++++++++++
"clusterThis" mantra render geo/volume instancer
++++++++++++++++++++++++++++++++++++++++++++++++


// New features

// -Usage of an Up vector combined with N or an orient attribute or rot.
// -Geometry file name per point
// -More control on motion blur, i.e. centered, backward and forward motion blur.




***BETA CODE - NO WARRANTY***

The current set of parameters are:
(Those marked with a * are either not yet implemented or are enabled in the pro version)
Setup Tab:
Min/Max Bounds 	The bounding box of the objects/volume being instanced
Type 	The type of geometry to instance
Num. Copies 	Number of copies of the geometry to instance per point of input geometry
Recursion 	Number of copies of the geometry per copy per point of input geometry
Radius 	The distance from the center of the point to the instanced geometry
Size: 	The size (X, Y, Z) of the instanced geometry primitive
Frequency 	Frequency X,Y,Z
Offset 	Offset X,Y,Z
Instance Probability 	Controls the amount of "randomness" in choosing to instance an object or not.
Geometry File 	Geometry disk file to be instanced
Deferred Instancing 	This will generate additional VRAY_Procedural objects, which then instance the geometry as needed by mantra. If unchecked, all the
geometry will be instanced at one time
Motion Blur 	Motion blur type: None, Velocity or Deformation
Shutter 	The shutter value for motion blur
Messages 	The verbosity of the messages sent to the console during rendering

Attributes Tab:
Copy Attributes * 	Copy attributes from source points to instanced primitives
Blend Attributes * 	Interpolate and filter attributes from source points to instanced primitives

Noise Tab:
Type 	Simple noise generator noise types Perlin, Sparse, Alligator
Amplitude 	Amplitude of noise applied to the position of the instanced geometry Noise Roughness: The "roughness" of the noise function
Roughness 	Roughness of the generated noise
Fractal Depth 	Depth of recursion for the noise generator
Attenuation 	Amount of attenuation applied to the noise generator
Seed 	Seed for the noise generator

CVEX Tab:
CVEX Shader 	The file name of the .vex shader to be used
Execute VEX Code 	Execute the VEX code on the instanced objects
CVEX Vars (point) 	The point attributes sent to the CVEX code for processing
CVEX Vars (primitive) 	The primitive attributes sent to the CVEX code for processing

Misc Tab:
Temp File Path 	Temporary file for the instanced geometry
Use temp geo cache file
Save the geometry in a temp cache file (for optimization)
Save temp cache file: 	Do not delete cache file (to save the temp geometry cache file for later use)

Example HIP file:
VRAY_clusterThis_dev_v.1.4.7.hip


INSTALLATION:
To install the "clusterThis" mantra procedural DSO, there are 3 components that need to be installed:

1) The "executable" that runs when mantra calls the clusterThis functions
2) The VRAYprocedural configuration file to tell mantra where to find the clusterThis DSO or DLL
3) The OTL, which is the interface to the mantra geometry procedural allowing the user to control it's behaviour


Step 1:
* The "executables":
VRAY_clusterThis.so
VRAY_clusterThis.dylib

Place one of these (depending on operating system) in a typical mantra DSO/DLL location, e.g.:
$HOME/houdiniX.XX/dso/mantra/
$JOB/houdiniX.XX/dso/mantra/


Step 2:
* The mantra procedural configuration file:
VRAYprocedural

Place this file (or append the contents to your current VRAYprocedural file) in a location searched by Houdini:
$HOME/houdiniX.XX/
$JOB/houdiniX.XX/


Step 3:
* Geometry Shader OTL:
VM_GEO_clusterThis.otl

Place this file in a location searched by Houdini:
$HOME/houdiniX.XX/otls/
$JOB/houdiniX.XX/otls/


NOTES:
The shader applied to the instanced geometry is currently the material defined at the object level, unless there's a "vm_surface" point attribute present, which will be used instead.


DEBUG TIPS:

1) To make sure the clusterThis DSO/DLL is available to mantra, from a UNIX (or CygWIN on Windows) shell, execute the following command:

earth:~> mantra -V4
Registering procedural 'clusterThis'
Registering procedural 'sprite'
Registering procedural 'image3d'
Registering procedural 'metapoly'
Registering procedural 'file'
Registering procedural 'fur'
Registering procedural 'program'
Registering procedural 'hscriptinstance'
Registering procedural 'image3dvolume'
mantra Version 9.5.169 (Compiled on 07/17/08)


The verbosity level of 4 or greater for mantra will report which mantra procedural DSO's are available.
If clusterThis is not in this list, the DSO is not in the right location, or the VRAYprocedural file isn't correct.




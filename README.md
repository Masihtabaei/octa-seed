# OctaSeed

A visually rich garden scene, even with lush trees and detailed foliage, often feels incomplete without fruits.
This paper builds on our previous work on procedural grass, leaf, and tree generation by introducing a method for procedurally generating fruits.
We developed parameterized mesh shaders to create fruits with a spherical shape.
As a base topology, we experimented with various options, including an octahedron, which is normalized and transformed into a sphere.
This sphere is then deformed into different fruit shapes using parameterized curves.
Different types of \ac{LOD} are also available to control quantity and also quality of fruits present in the scene.


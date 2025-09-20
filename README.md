# OctaSeed

> [!NOTE]  
> This project was completed as part of the **GPU Image Synthesis** course during the **Winter Term 2025** of our undergraduate studies, offered by [Prof. Dr. Quirin Meyer](https://www.hs-coburg.de/en/personen/prof-dr-quirin-meyer/) at [Coburg University](https://www.hs-coburg.de/en/). It was originally developed on our university’s GitLab instance and later partially migrated here, so some elements like issues may be missing.

> [!IMPORTANT]  
> This project is licensed under the [MIT License](https://masihtabaei.dev/licenses/mit).

> [!TIP]
> This project was later integrated in our award-winning publication. You can see it in action [here](https://github.com/Bloodwyn/gptree) and find the publication [here](https://diglib.eg.org/items/93fc78c0-71fa-4511-8564-a7e5268bf27a).

## General Information

Sueprvisors: [Prof. Dr. Quirin Meyer](https://www.hs-coburg.de/en/personen/prof-dr-quirin-meyer/) and [Bastian Kuth](https://bloodwyn.github.io/#CV_Bastian_Kuth.pdf)

In this work, we introduce OctaSeed, a novel method for procedural three-dimensional (3D) fruit generation that leverages the
power of mesh shaders. Our approach enables the efficient creation of fruits that can be represented as spheres or derived from
spherical forms, supporting both inter- and intra-level of details (LODs). As the base topology, we use an octahedron, which is
first deformed into an unit sphere and then further customized using parametric cubic Bézier curves. This allows us to generate
a diverse range of fruit shapes, including apples, pears, strawberries, and lemons, with high geometric flexibility and efficiency.


## Usage

```bash
git clone <repo-link>
cd octa-seed
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=<vcpkg.cmake-file-path> ..
```

After generating the solution, you can open it and select and run one of the projects within the `Assignments`-directory to generate some fruits. 

## Screenshots

<img width="400" height="400" alt="per-face-normal" src="https://github.com/user-attachments/assets/baf67da0-8234-4f4a-b91f-9b988ae1f331" />
<img width="400" height="400" alt="per-pixel-normal" src="https://github.com/user-attachments/assets/93d65066-dccb-41c2-a64e-dc65acd529b2" />
<img width="400" height="400" alt="apple" src="https://github.com/user-attachments/assets/01315c9c-71c3-46ce-8839-e7e7575e582f" />
<img width="400" height="400" alt="apple-wireframe" src="https://github.com/user-attachments/assets/454a4647-6fc3-43ba-a148-4cfd0c7d75d3" />
<img width="400" height="400" alt="lemon" src="https://github.com/user-attachments/assets/0072de5b-2e42-4ae0-878d-25871375878c" />
<img width="400" height="400" alt="pear" src="https://github.com/user-attachments/assets/edd956d0-5d58-4985-b68a-403b7b2d419b" />
<img width="400" height="400" alt="strawberry" src="https://github.com/user-attachments/assets/5c5b6ecd-3444-4e65-b473-8b182a96ed70" />
<img width="400" height="400" alt="strawberry-wireframe" src="https://github.com/user-attachments/assets/85843eeb-48bb-485e-a298-65fefd2b8ca7" />


<!doctype html>
<html>
<head>
  <title>Real-Time Deformable Terrain</title>
  
  <link rel="stylesheet" href="style.css" type="text/css" media="screen" /> 
</head>
<body>
<div id="container">
  <!------------------------------------------------------------->
  <!---- BANNER ---->
  <!------------------------------------------------------------->
  <header id="banner">
  </header>
  <!------------------------------------------------------------->
  <!---- NAVIGATION ---->
  <!------------------------------------------------------------->
  <nav>
    <ul>
      <li><a href="index.php">    	Home     </a></li>
      <li><a href="details.php">    Details  </a></li>
      <li><a href="gallery.php">    Gallery  </a></li>
      <li><a href="downloads.php">  Downloads</a></li>
      <li><a href="contact.php">    Contact  </a></li>
    </ul>
  </nav>

  <!------------------------------------------------------------->
  <!---- TEASER ---->
  <!------------------------------------------------------------->
  <section id="teaser">
    <header>
    </header>
  </section>
  
  <!-- start content -->
  <div id="content">
  
    <!------------------------------------------------------------->
    <!---- MAIN CONTENT ---->
    <!------------------------------------------------------------->
    <div id="mainContent">
      <section id="sectGroup">
        <!-- Overview -->
        <section>
          <a name="overview"></a>
          <header>
            <h2>Project Overview</h2>
            <p>
              A Framework for Real-Time Deformable Terrain
            </p>
          </header>
          
          <div>
            <p>
              This project is about sex, and sex alone. Sex is good, i hear
            </p>
          </div>
          <a href="#">&uarr;Top&uarr;</a>
        </section> <!-- end overview -->
        
        <!-- Geom Tessellation -->
        <section>
          <a name="geomtess"></a>
          <header>
            <h2>Geometry Tessellation Technique</h2>
            <p>
              Representation of high-detail by displacing a tessellated mesh
            </p>
          </header>
          
          <div>
            <p>
              The first method implemented for the representation of high-detail deformations uses displacement mapping
on a finer scale. Due to the coarse granularity of the raw clipmap mesh, simply displacing the
existing vertices yields no extra detail. For this reason, the existing mesh is further tessellated, or refined,
such that each triangle is subdivided into 9 sub-triangles. The resulting vertices are then displaced
according to the high-detail data. High-detail is only considered in regions close to the camera and only
the inner grid surrounding the camera is thus tessellated. The perfomance of the Geometry Shader drops roughly linearly with the number of output elements and it therefore crucial to minimise the number of output triangles. For this reason, adaptive tessellation is employed whereby areas are only refined if they contain high detail. Although this improves the average performance of the tessellation method, the worst-case is the same as that of a non-adaptive method where all triangles (within the inner grid) are tessellated regardless of the detail.</p>
</div>
          <a href="#">&uarr;Top&uarr;</a>
        </section>  <!-- end geomtess -->
        
        <!-- Parallax -->
        <section>
          <a name="parallax"></a>
          <header>
            <h2>Texture-Based Technique</h2>
            <p>
              Representation of high-detail using Parallax Mapping
            </p>
          </header>
          
          <div>
            <p>
              This implementation relies on texture trickery techniques such as normal and parallax mapping. The
aim of this implementation is to produce the illusion of high-detail on the terrain without the overhead
of creating additional geometry. This should allow for unlimited deformations to occur without
any noticeable slow-down since there will be always the same amount of geometry in the scene. The
process involves more texture reads and a more complicated lighting calculation; these overheads will
reduce the performance of application. However, this penalty is constant irrespective of the number
of deformations. This effect is purely illusionary and as a result has certain limitations that must be
managed to prevent artefacts becoming noticeable. This implementation will compete directly with the
aforementioned geometry tessellation one.
            </p>
          </div>
          <a href="#">&uarr;Top&uarr;</a>
        </section>  <!-- end parallax -->
        
        <!-- Interface -->
        <section>
          <a name="interface"></a>
          <header>
            <h2>Computer Vision Interface</h2>
            <p>
              Interface for terrain manipulation via hand movement bleh bleh.
            </p>
          </header>
          
          <div>
            <p>
              This component forms the front-end to the two separate back-ends described above. This interface is a
computer vision based system wherein the user interacts with the application directly through the use
of gestures. This component is designed to integrate easily with the two deformation components. The
vision system uses object tracking, background subtraction and minimal use of hand pose estimation to
create a functional input device from the users hand, hand occlusion, pose and position relative to the
computer monitor.
            </p>
          </div>
          <a href="#">&uarr;Top&uarr;</a>
        </section>  <!-- end interface -->
        
      </section>  <!-- end sectGroup -->
    </div> <!-- end mainContent -->
      
    <!------------------------------------------------------------->
    <!---- SIDEBAR CONTENT ---->
    <!------------------------------------------------------------->
    <aside>
      <section id="sublinks">
        <ul>
          <li>
            <a href="#overview">Overview</a>
          </li>
          <li>
            <a href="#geomtess">Geometry Tessellation</a>
          </li>
          <li>
            <a href="#parallax">Texture-Based</a>
          </li>
          <li>
            <a href="#interface">Vision Interface</a>
          </li>
         </ul>
      </section>
    </aside>
  </div> <!-- end content -->
  <footer>
    <!-- Footer -->
  </footer>

</div> <!-- end container -->

</body>
</html>
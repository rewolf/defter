<!doctype html>
<html>
<head>
  <title>Real-Time Deformable Terrain</title>
  
  <link rel="stylesheet" href="style.css" type="text/css" media="screen" />
  
  <script type="text/javascript">
    function mouseEvent(id, state)
    {
      document.getElementById(id).src="images/buttons/"+id+state+".png";
    }
  </script>

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
      <li>
        <a href="index.php">
          <img src="images/buttons/home.png" id="home"
           onMouseOver="mouseEvent('home', '-hover')"
           onMouseDown="mouseEvent('home', '-pressed')"
           onMouseOut="mouseEvent('home', '')"
           onMouseUp="mouseEvent('home', '-hover')"
          >
        </a>
      </li>
      <li>
        <a href="#">
          <img src="images/buttons/details.png" id="details"
           onMouseOver="mouseEvent('details', '-hover')"
           onMouseDown="mouseEvent('details', '-pressed')"
           onMouseOut="mouseEvent('details', '')"
           onMouseUp="mouseEvent('details', '-hover')"
          >
        </a>
      </li>
      <li>
        <a href="gallery.php">
          <img src="images/buttons/gallery.png" id="gallery"
           onMouseOver="mouseEvent('gallery', '-hover')"
           onMouseDown="mouseEvent('gallery', '-pressed')"
           onMouseOut="mouseEvent('gallery', '')"
           onMouseUp="mouseEvent('gallery', '-hover')"
          >
        </a>
      </li>
      <li>
        <a href="downloads.php">
          <img src="images/buttons/downloads.png" id="downloads"
           onMouseOver="mouseEvent('downloads', '-hover')"
           onMouseDown="mouseEvent('downloads', '-pressed')"
           onMouseOut="mouseEvent('downloads', '')"
           onMouseUp="mouseEvent('downloads', '-hover')"
          >
        </a>
      </li>
      <li>
        <a href="contact.php">
          <img src="images/buttons/contact.png" id="contact"
           onMouseOver="mouseEvent('contact', '-hover')"
           onMouseDown="mouseEvent('contact', '-pressed')"
           onMouseOut="mouseEvent('contact', '')"
           onMouseUp="mouseEvent('contact', '-hover')"
          >
        </a>
      </li>
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
              The goal of this project is to create a terrain deformation test-bed that supports the real-time deformation of a triangle mesh representing a terrain. The system must be capable of handling a large number of deformations whilst maintaining real-time frame-rates. Ultimately this system will serve as basis for creating realistically deforming terrain environment for use in computer games and visual effects. While methods do exist to change geometry on-the-fly, these have not been widely adopted in computer games mainly due to previous limits imposed by graphics hardware. Currently most computer games rely on pre-computed data, meaning that the terrain (or other environmental objects) can only be changed in a small number of predefined ways, thus breaking immersion.
            </p>
            <p>
              The work of the project is divided up into three components of equal weight. The first and second components share a common framework which is collaboratively developed by Andrew Flower and Justin Crause. This common framework allows for coarse level deformations to take place on the terrain, the implementation of the caching system. The core application also includes the representation of the terrain mesh and basic rendering functionality. The main distinction between the two components comes about with the addition of high-detail deformations to the terrain, one uses <a href="#geomtess">Geometry Tessellation</a> and the other uses a <a href="#parallax">Texture-Based</a> approach. These two back-end implementations will be combined with a  <a href="#interface">Vision Interface</a> front-end to create a unique real-time terrain deformation system.
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
              Representation of high-detail by further mesh tessellation
            </p>
          </header>
          
          <div>
            <p>
              The first method implemented for the representation of high-detail deformations uses displacement mapping on a finer scale.  Due to the coarse granularity of the raw clipmap mesh, simply displacing the existing vertices yields no extra detail.  For this reason, the existing mesh is further tessellated, or refined, such that each triangle is subdivided into 9 sub-triangles.  The resulting vertices are then displaced according to the high-detail data.  High-detail is only considered in regions close to the camera and only the inner grid surrounding the camera is thus tessellated.  Ideally, tessellation within this grid would be adaptive, in that regions would only be tessellated if high-detail data existed.   Instead, as this implementation currently stands, tessellation is performed for the entire inner grid within a specified radius.  This method does however yield consistent performance hits.  The performance of an adaptive approach would decrease in areas of much high-detail, although the average-case performance would be considerably better.
            </p>
</div>
          <a href="#">&uarr;Top&uarr;</a>
        </section>  <!-- end geomtess -->
        
        <!-- Parallax -->
        <section>
          <a name="parallax"></a>
          <header>
            <h2>Texture-Based Technique</h2>
            <p>
              Representation of high-detail using texture-based methods
            </p>
          </header>
          
          <div>
            <p>
              This implementation relies on texture trickery techniques such as normal and parallax mapping. The aim of this implementation is to produce the illusion of high-detail on the terrain without the overhead of creating additional geometry. This should allow for unlimited deformations to occur without any noticeable slow-down since there is always the same amount of geometry in the scene. The process involves more texture reads and a more complicated lighting calculation; these overheads reduce the performance of application. However, this penalty is constant irrespective of the number of deformations. This effect is purely illusionary and as a result has certain limitations that must be managed to prevent artefacts becoming noticeable. This implementation competes directly with the aforementioned geometry tessellation one.
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
              Interface for terrain manipulation via hand gestures
            </p>
          </header>
          
          <div>
            <p>
              This component forms the front-end to the two separate back-ends described above. This interface is a computer vision based system wherein the user interacts with the application directly through the use of gestures. This component is designed to integrate easily with the two deformation components. The vision system uses object tracking, background subtraction and minimal use of hand pose estimation to create a functional input device from the users hand, hand occlusion, pose and position relative to the computer monitor.
The contents of this report focus on component two on texture-based methods. For information pertaining to one of the other components please see the respective documentation.
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
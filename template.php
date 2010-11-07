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
        <a href="details.php">
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
        <section>
          <header>
            <h2>Section Title</h2>
            <p>
              Section Subtitle
            </p>
          </header>
          <div>
            <p>
            First paragraph of Section
            </p>
            <p>
            Second paragraph of Section
            </p>
          </div>
        </section>
      </section>
    </div> <!-- end mainContent -->
      
    <!------------------------------------------------------------->
    <!---- SIDEBAR CONTENT ---->
    <!------------------------------------------------------------->
    <aside>
      Sidebar Content, such as sub-links. <br/>
      Delete this <i>aside</i> tag if you don't need sidebar.
    </aside>
  </div> <!-- end content -->
  <footer>
    <!-- Footer -->
  </footer>

</div> <!-- end container -->

</body>
</html>

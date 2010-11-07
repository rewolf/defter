<!doctype html>
<html>
<head>
  <title>Real-Time Deformable Terrain</title>
  
  <script src="js/jquery-1.3.2.min.js" type="text/javascript"></script>
  <script src="js/jquery.prettyPhoto.js" type="text/javascript" charset="utf-8"></script>
  
  <link rel="stylesheet" href="css/prettyPhoto.css" type="text/css" media="screen" title="prettyPhoto main stylesheet" charset="utf-8" />
  <link rel="stylesheet" href="style.css" type="text/css" media="screen" /> 
  
	<?php
		// Include code used to insert gallery images
    include("util.php");
  ?>
  
</head>
<body>
<div id="container">
  <!------------------------------------------------------------->
  <!---- BANNER ---->
  <!------------------------------------------------------------->
  <header>
    <h1>BRUCE BANNER</h1>
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
      <li><a href="contact.php">		Contact  </a></li>
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
      	<!-- Development Shots -->
        <section>
          <header>
            <h2>Development</h2>
            <p>
              FUNNESS!!!!
            </p>
          </header>
          <div>
          	<?php
							insert_gallery("development");
						?>
          </div>
        </section>
        
        <!-- Videos -->
        <section>
          <header>
            <h2>Videos</h2>
            <p>
              FUNNESS!!!!
            </p>
          </header>
          <div>
          	<?php
							insert_gallery("videos");
						?>
          </div>
        </section>
      </section>
    </div> <!-- end mainContent -->
  
  </div> <!-- end content -->
  <footer>
    <!-- Footer -->
  </footer>

</div> <!-- end container -->

<script type="text/javascript" charset="utf-8">
  $(document).ready(function(){
    $("a[rel^='prettyPhoto']").prettyPhoto({animationSpeed:'slow',
                        theme:'light_square',
												slideshow:4000,
												autoplay_slideshow: true});
  });
</script>

</body>
</html>

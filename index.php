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
        <a href="#">
          <img src="images/buttons/home.png" id="home"
           onMouseOver="mouseEvent('home', '-hover')"
           onMouseDown="mouseEvent('home', '-pressed')"
           onMouseOut="mouseEvent('home', '')"
           onMouseUp="mouseEvent('home', '-hover')"
           class="selected"
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
            <h2>Welcome</h2>
            <p>
              ..to the home of Project DefTer
            </p>
          </header>
          <div class="twocolumn">
            <p>
            <!--
            	focus of site
              honours project
            -->
            	Project DefTer is a project developed by a team of three very smashing individuals.  This project
              was allocated as the mandatory Honours project given to Computer Science Honours students at the
              <a href="http://www.uct.ac.za/">University of Cape Town</a>.   The purpose of this site is to 
              present this work.  The Gallery page contains a collection of images and videos pertaining to the
              development of the Deformable Terrain system.  The downloads page lists the reports and research
              documents compiled by each team member as well as the team's original proposal.  Detail regarding
              individual members and the supervisor, Dr Patrick Marais, can be found in the Contact page.
            </p>
            <p>
            <!--
            	project details (coarsemap)
              no-decay
              2-levels
            -->
              The Real-Time Deformable Terrain system (codename DefTer) was developed in order to mend a problem
              with modern computer games.  The focus of most computer games is realism, such that players may
              actually feel present within the environment.  One of the first things a player does is test the 
              limits of a game environment.  When an action is performed, it logical to expect a reaction.  The
              terrain of most modern computer games is static, however, and yields no appropriate reaction to
              gunshots or explosions for example.   The DefTer system removes this limitation, providing a 
              framework for a dynamic terrain that can be modified in real-time by in-game events, thereby 
              increasing the realism of games and allowing users more freedom.  The modifications performed 
              persist for the duration of the game and even across executions, unlike the decal-based detail
              seen in most games. 
              Such an environment allows for
              countless new features such as bullet (or explosion) -caused craters, footprints that allow 
              opponents to track the player and the ability to dig trenches or burrow under enemy barricades.
            </p>
            <p>
            <!--
            	introduce team
            -->
            	Team DefTer consists of Andrew Flower, Justin Crause, Peter Juritz and is supervised by Dr Patrick
              Marais.  Each member is responsible for an independent subsystem of the project.  The DefTer 
              system provides two levels for deformation.  The method of presentation of the fine-level detail
              was approached from two different angles.  These two implementations were developed by Andrew and
              Justin.  As an alternative to the familiar mouse-based interface, Peter developed a computer 
              vision interface allowing a user to manipulate the terrain by merely placing his/her hand in front
              of the screen and performing specific gestures or direct manipulation.
            </p>
            
          </div>
        </section>
      </section>
    </div> <!-- end mainContent -->
      
		<!------------------------------------------------------------->
		<!---- SIDEBAR CONTENT ---->
		<!------------------------------------------------------------->
		<aside>
			
		</aside>
  </div> <!-- end content -->
  <footer>
    <!-- Footer -->
  </footer>

</div> <!-- end container -->

</body>
</html>

<!doctype html>
<html>
<head>
  <title>Real-Time Deformable Terrain</title>
  
  <link rel="stylesheet" href="style.css" type="text/css" media="screen" />
  
  <script language="javascript">
      var contacts = new Array("af", "jc", "pj", "pm");
      var curIntensity = 0;
      var changer = +2;
      function viewContact(name){
      // insert the appropriate text
      var source = document.getElementById(name);
      var target = document.getElementById("contactInfoBox");
      target.innerHTML = source.innerHTML;
      for (var i =0 ; i < 4 ; i++){
        var el = document.getElementById(contacts[i]+"img");
        if (contacts[i] != name){
          el.setAttribute("class","imgOffBox");
          el.style.border = "solid 2px rgb(0,0,0,0)";
        }
        else
          el.setAttribute("class","imgOnBox");
          el.style.border = "solid 2px rgb(0,0,0,0)";
      }
    }
      
    function flash(){
      var active = document.getElementById("contactInfoBox");
      if (active != null){
        curIntensity = curIntensity + changer;
        if (curIntensity > 200 || curIntensity < 0)
          changer = - changer;
        active.style.border = "solid 2px rgb(0,0,"+curIntensity+")";
      }
    }
	
    function mouseEvent(id, state)
    {
      document.getElementById(id).src="images/buttons/"+id+state+".png";
    }
    
    setInterval("flash()", 20);
  </script>
  
</head>
<body onLoad="viewContact('af');">
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
        <a href="#">
          <img src="images/buttons/contact.png" id="contact"
           onMouseOver="mouseEvent('contact', '-hover')"
           onMouseDown="mouseEvent('contact', '-pressed')"
           onMouseOut="mouseEvent('contact', '')"
           onMouseUp="mouseEvent('contact', '-hover')"
           class="selected"
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
            <h2>Contact Details</h2>
          </header>
          <div id="contactImgBox">
            <div class="imgOffBox" id="afimg">
              <img src="images/andrew.jpg"   id="afimg" width=120 onMouseOver="viewContact('af');" />
            </div>
            <div class="imgOffBox" id="jcimg">
              <img src="images/justin.jpg"   id="jcimg" width=120 onMouseOver="viewContact('jc');" />
            </div>
            <div class="imgOffBox" id="pjimg">
              <img src="images/peter.jpg"   id="pjimg" width=120 onMouseOver="viewContact('pj');" />
            </div>
            <div class="imgOffBox" id="pmimg">
              <img src="images/patrick.jpg"  id="pmimg" width=120 onMouseOver="viewContact('pm');" />
            </div>
          </div>          
          <div id="contactInfoBox">
          </div>
          
          <div id="af" class="contactInfo">
            <h3>Andrew Flower</h3>
            <p class="role">
              Tessellation Scheme<br/>
            </p>
            <p>
              Email: andrew.flower [at] gmail.com<br/>
              Website: <a href="../andrew/index.html">Click here</a>
            </p>
          </div>
          
          <div id="jc" class="contactInfo">
            <h3>Justin Crause</h3>
            <p class="role">
              Texture-Based Technique
            </p>
            <p>
              Email: juzzwuzz [at] gmail.com<br/>
              Website: <a href="../justin/index.html">Click here</a>
            </p>
          </div>
          
          <div id="pj" class="contactInfo">
            <h3>Peter Juritz</h3>
            <p class="role">
              Computer Vision Interface
            </p>
            <p>
              Email: peter.juritz [at] gmail.com<br/>
              Website: <a href="../peter/index.html">Click here</a>
            </p>
          </div>
          
          <div id="pm" class="contactInfo">
            <h3>Dr Patrick Marais</h3>
            <p class="role">
              Supervisor
            </p>
            <p>
              Email: patrick [at] cs.uct.ac.za<br/>
              Website: <a href="http://people.cs.uct.ac.za/~patrick">Click here</a>
            </p>
          </div>
        </section>
      </section>
    </div> <!-- end mainContent -->
      
    <!------------------------------------------------------------->
    <!---- SIDEBAR CONTENT ---->
    <!------------------------------------------------------------->
    <aside>
      <section id="qr-code">
        <img src="images/qr.png" width="180"/>
      </section>
    </aside>
  </div> <!-- end content -->
  <footer>
    <!-- Footer -->
  </footer>

</div> <!-- end container -->

</body>
</html>

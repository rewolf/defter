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
			var target = document.getElementById("contactBox");
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
			var active = document.getElementById("contactBox");
			if (active != null){
				curIntensity = curIntensity + changer;
				if (curIntensity > 200 || curIntensity < 0)
					changer = - changer;
				active.style.border = "solid 2px rgb(0,0,"+curIntensity+")";
			}
		}
		
		setInterval("flash()", 20);
	</script>
</head>
<body onload="viewContact('pm');">
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
        <section>
          <header>
            <h2>Contact Details</h2>
          </header>
          <div id="contactImgBox">
          	<div class="imgOffBox" id="afimg">
              <img src="images/andrew.jpg" 	id="afimg" width=120 onmouseover="viewContact('af');" />
            </div>
          	<div class="imgOffBox" id="jcimg">
              <img src="images/justin.jpg" 	id="jcimg" width=120 onmouseover="viewContact('jc');" />
            </div>
          	<div class="imgOffBox" id="pjimg">
              <img src="images/peter.jpg" 	id="pjimg" width=120 onmouseover="viewContact('pj');" />
            </div>
          	<div class="imgOffBox" id="pmimg">
              <img src="images/patrick.jpg"	id="pmimg" width=120 onmouseover="viewContact('pm');" />
            </div>
          </div>          
          <div id="contactBox">
          </div>
          <div id="af" class="contactInfo">
            <h3>Andrew Flower</h3>
            <p>
            role: tessellation scheme<br/>
            email: andrew dot flower at gmail dot com<br/>
            website: http://rewolf.dyndns.org
            </p>
          </div>
          <div id="jc" class="contactInfo">
            <h3>Justin Crause</h3>
            <p>
            role: parallax mapping scheme<br/>
            email: juzzwuzz at gmail dot com
            </p>
          </div>
          <div id="pj" class="contactInfo">
            <h3>Peter Juritz</h3>
            <p>
            role: computer vision interface<br/>
            email: peter dot juritz at gmail dot com
            </p>
          </div>
          <div id="pm" class="contactInfo">
            <h3>Dr Patrick Marais</h3>
            <p>
            role: supervisor<br/>
            email: peter dot juritz at gmail dot com
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

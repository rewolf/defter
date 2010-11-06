<!doctype html>
<html>
<head>
	<title>Real-Time Deformable Terrain</title>
  
	<script src="js/jquery-1.3.2.min.js" type="text/javascript"></script>
  <script src="js/jquery.prettyPhoto.js" type="text/javascript" charset="utf-8"></script>
  
  <script type="text/javascript" charset="utf-8">
		$(document).ready(function(){
			$("a[rel^='prettyPhoto']").prettyPhoto({animationSpeed:'slow',
					theme:'light_square',slideshow:4000, autoplay_slideshow: true});
		});
  </script>
  <link rel="stylesheet" href="css/prettyPhoto.css" 
  			type="text/css" media="screen" title="prettyPhoto main stylesheet" charset="utf-8" />
  <link rel="stylesheet" href="style.css" type="text/css" media="screen" /> 
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
      <li><a href="index.php">		Home			</a></li>
      <li><a href="gallery.php">	Gallery		</a></li>
      <li><a href="details.php">	Details		</a></li>
      <li><a href="#">						Downloads	</a></li>
      <li><a href="#">						Contact		</a></li>
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
            <h2>Gallery</h2>
            <p>
              Shows the development photos. bleh. not sure how you want this all laid out, so this is just a temporary layout. have fun JuzzWuzz
            </p>
          </header>
          <div>
    <?php
			class Item{
				public $filename = '';
				public $title    = NULL;
				public $caption  = NULL;
			}
			
			$dir = "gallery/development";
			
			$fp = fopen("$dir/_details.txt", "r");
			
			$line = fgets($fp);
			$items = array();
			$cur_item = NULL;	
			$data_ctr = 0;
			// parse lines of file
			while ($line != FALSE){
				// ignore comments
				if ($line[0] == '#' || strlen(trim($line))==0){
					$line = fgets($fp);
					continue;
				}
				if ($line[0] == '$'){
					if ($data_ctr == 0)
						$cur_item->title = trim($line,"\n$");
					else
						$cur_item->caption = trim($line, "\n$");
					$data_ctr = $data_ctr + 1;
				}
				else{
					if ($cur_item != NULL)
						$items[] = $cur_item;
					$cur_item = new Item();
					$cur_item->filename = trim($line);
					$data_ctr = 0;
				}
				$line = fgets($fp);
			}
			fclose($fp);
			
			// Output the HTML for each item
			echo "\n<ul class=\"imgList\">\n";  				// starts a list
			foreach ($items as $i => $item){
				echo "  <li>\n";			// start list element
				echo "    <a href=\"{$dir}/{$item->filename}\" rel=\"prettyPhoto[gallery1]\"";
				if ($item->caption != NULL)
					echo " title=\"{$item->caption}\"";
				echo ">\n";
				echo "      <img src=\"{$dir}/thumbs/{$item->filename}\"";
				if ($item->title != NULL)
					echo " alt=\"{$item->title}\"";
				echo "/>\n";
				echo "    </a>\n";
				echo "</li>\n";				// end list element
			}
			echo "</ul>\n";					// end list
			
		?>
          </div>
        </section>
      </section>
    </div> <!-- end mainContent -->
      
  </div> <!-- end content -->
	<footer>
	</footer>

</div> <!-- end container -->

</body>
</html>

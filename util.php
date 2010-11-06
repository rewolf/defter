<?php

// Item class used when reading the _details.txt files
class Item{
	public $filename = '';
	public $title    = NULL;
	public $caption  = NULL;
}

// Used by Gallery to insert a list of lightbox images
function insert_gallery($dir){
	$dir = "gallery/$dir";
	
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
	if ($cur_item != NULL)
		$items[] = $cur_item;
	fclose($fp);
	
	// Output the HTML for each item
	echo "\n<ul class=\"imgList\">\n";    // starts a list
	foreach ($items as $i => $item){
		echo "  <li>\n";    // start list element
		echo "    <a href=\"{$dir}/{$item->filename}\" rel=\"prettyPhoto[gallery1]\"";
		if ($item->caption != NULL)
			echo " title=\"{$item->caption}\"";
		echo ">\n";
		echo "      <img src=\"{$dir}/thumbs/{$item->filename}\"";
		if ($item->title != NULL)
			echo " alt=\"{$item->title}\"";
		echo " />\n";
		echo "    </a>\n";
		echo "  </li>\n";    // end list element
	}
	echo "</ul>\n";    // end list
}
?>
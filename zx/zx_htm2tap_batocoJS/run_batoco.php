
<html lang="en">
<!--
-230122 - Added PC number to submission form
	-->
  <head>
    <meta charset="utf-8">

	<script type="text/javascript">

	//this.onload = function(){window.open($tmpURL; , "emulator_output");};
	
	function load()
	{ //alert ("hello");
		//load2();
		setTimeout(function() {   		load2(); 
		}, 1000);
		
		//history.back() ; //history.go(-1)
	        
	}

	</script>

   </head>
   <body onload="load()" >

<?php
include "batoco.php";
$target_subfolder="word_saved_data/";
//$qaopURL=$target_subfolder.'QAOP/qaop.html';
$qaopURL='qaop.html';
//$file_name="sch".$_REQUEST["o"]."-".$_REQUEST["taksi"].$_REQUEST["tmima"]."-".date('Ymd')."-PC".$_REQUEST["pc"]."-".$_REQUEST["name"];
$file_name=$_REQUEST["o"];

?>
<?php 
 echo '
	<script type="text/javascript">

	//this.onload = function(){window.open($tmpURL; , "emulator_output");};
	
	function load2()
	{ //alert ("hello2");
		//window.open( echo $tmpURL , "emulator_output"); ';
		//$tmpURL2='"'.$qaopURL."?#l=".$target_subfolder."$file_name.tap\"";
		//$tmpURL2='"'.$qaopURL."?#l=".$target_subfolder."$file_name.tap\"";
		$tmpURL2='"'.$qaopURL."?#l=$file_name\"";
	echo '	
		//window.open("https://www.w3schools.com");
		window.open(  '.$tmpURL2.'  , "emulator_output");
		//history.back() ; //history.go(-1)
	        
	}

	</script>
	'; 
	?>
</body> 
</html>

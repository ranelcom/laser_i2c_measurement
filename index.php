<html>
 <head>
  <title>EFE</title>
 </head>
 <body>
 <H1 align="center"> <b>EMPRESA DE FERROCARRILES DE CHILE</b> </H1>	 
 <H2 align="center"> <b>MONITOREO PUENTE CAUTIN</b> </H2>
 <br><br>
 <p align="center"> 
 <b>LECTURA LASER ACTUAL</b> 
 <?php
 echo file_get_contents( "/home/pi/registro_distancia_actual" ); // get the contents, and echo it out.
 $registro_aux = file_get_contents( "/home/pi/log.log" );
 $registro = str_replace("!", "<br>", $registro_aux);
 echo "<br><br><b>ULTIMOS REGISTROS</b><br>" . $registro . "<br>"; 
 ?>
 </p>
 <br><br><br>
 <p align="center">ESETEL S.P.A.</p>
 <p align="center">www.esetel.cl</p>	 
 
 </body>
</html>




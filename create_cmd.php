<?PHP

$row = 1;
if (($handle = fopen("control_cmd.csv","r"))!==FALSE) {
    while (($data = fgetcsv($handle, 1000, "\t")) !== FALSE) {
        $num = count($data);
		$cmd=trim($data[0]);
		$bytes=trim($data[1]);
		$numBytes=trim($data[2]);
		$bytes=str_getcsv($bytes," ");
		echo count($bytes)."|".$numBytes."\n";
		$package='';
		$fp=fopen("./data/".$cmd.".dat","w");
		foreach ( $bytes as $byte ) {
			$byte=substr($byte,-2);
			$v=hex2bin($byte);
			fwrite($fp,$v);
		}
		fclose($fp);
		
        $row++;
    }
    fclose($handle);
}

?> 

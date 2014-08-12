<?php

/**
* rrmdir - Remove non-empty directories
*
* @see http://php.net/manual/en/function.rmdir.php#107233
*
* @param string $dir   Full path of directory to be deleted
*/
function rrmdir($dir) {
   if (is_dir($dir)) {
     $objects = scandir($dir);
     foreach ($objects as $object) {
       if ($object != "." && $object != "..") {
         if (filetype($dir."/".$object) == "dir") rrmdir($dir."/".$object); else unlink($dir."/".$object);
       }
     }
     reset($objects);
     rmdir($dir);
   }
 }

 /**
 * jsonpp - Pretty print JSON data
 *
 * In versions of PHP < 5.4.x, the json_encode() function does not yet provide a
 * pretty-print option. In lieu of forgoing the feature, an additional call can
 * be made to this function, passing in JSON text, and (optionally) a string to
 * be used for indentation. @see http://ryanuber.com/07-10-2012/json-pretty-print-pre-5.4.html
 *
 * @param string $json  The JSON data, pre-encoded
 * @param string $istr  The indentation string
 *
 * @return string
 */
function jsonpp($json, $istr='  ') {
  if (is_array($json) || is_object($json)) {
    $json = json_encode($json);
  }
  $result = '';
  for ($p=$q=$i=0; isset($json[$p]); $p++) {
    $json[$p] == '"' && ($p>0?$json[$p-1]:'') != '\\' && $q=!$q;
    if (strchr('}]', $json[$p]) && !$q && $i--) {
      strchr('{[', $json[$p-1]) || $result .= "\n".str_repeat($istr, $i);
    }
    $result .= $json[$p];
    if (strchr(',{[', $json[$p]) && !$q) {
      $i += strchr('{[', $json[$p])===FALSE?0:1;
      strchr('}]', $json[$p+1]) || $result .= "\n".str_repeat($istr, $i);
    }
  }
  return str_replace('\\/', '/', $result);
}

/**
* recurse_copy - Recusive copy
*
*
* @param string $src  Source directory to copy
* @param string $dst  Destination directory
*
* @return string
*/
function recurse_copy($src,$dst) {
  $dir = opendir($src);
  @mkdir($dst);
  while(false !== ( $file = readdir($dir)) ) {
    if (( $file != '.' ) && ( $file != '..' )) {
      if ( is_dir($src . '/' . $file) ) {
        recurse_copy($src . '/' . $file,$dst . '/' . $file);
      }
      else {
        copy($src . '/' . $file,$dst . '/' . $file);
      }
    }
  }
  closedir($dir);
}

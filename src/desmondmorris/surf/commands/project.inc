<?php


function _surf_init($config) {
  $templates_path = '/lib/templates';
  $templates_dir = SURF_PATH . $templates_path;

  $objects = new RecursiveIteratorIterator(new RecursiveDirectoryIterator($templates_dir),
  RecursiveIteratorIterator::SELF_FIRST);

  foreach($objects as $name => $object){

      $fileName = $object->getFileName();
      $pathName = $object->getPathName();
      $relative_path = str_replace($templates_dir, '', $pathName);

      if(in_array($fileName, array('.', '..'))) {
        continue;
      }

      if (is_dir($pathName)) {
        exec('mkdir -p .' . $relative_path);
      }
      else {

        preg_match_all('/PROJECT([A-Z]*)/', $fileName, $matches, PREG_SET_ORDER);

        if (!empty($matches)) {

          foreach ($matches as $match){
            if (isset($config[strtolower($match[1])])) {
              $fileName = str_replace($matches[0], $vars[strtolower($match[1])], $fileName);
              $relative_path = str_replace($matches[0], $vars[strtolower($match[1])], $relative_path);
            }
          }
        }

        if (substr($fileName, -8) == '.tpl.php') {
           extract($config);
           ob_start();
           include($pathName);
           $data = ob_get_contents();
           ob_end_clean();

           $filename = str_replace('.tpl.php', '', '.' . $relative_path);
           file_put_contents($filename, $data);
        }
        else {
          exec('cp ' . $pathName . ' .' . $relative_path);
        }
      }
  }
}

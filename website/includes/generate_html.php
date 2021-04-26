<?php


function generate_html($parameters){
    $color_red = "#E83800";


    $returnHTMLWithoutWhitespace = false;
    $http_host = $_SEVER["HTTP_HOST"];
    $rootPath = "//".$_SERVER["HTTP_HOST"]."/";

    $site_title = $parameters["site_title"];
    if($site_title == ""){
        $site_title = "HyQu";
    }


    $site_title = $parameters["site_title"];
    $html = $parameters["html"];


    if($parameters["linkback"] != ""){

        $link = explode("|",$parameters["linkback"]);

        $link_href = $link[0];
        $link_innerHTML = $link[1];
        $link_title = $link[2];

        if($link_title == ""){
            $link_title = $link_href;
        }

        $linkbackHTML = "<div class='linkBack'><a href='".$link_href."' class='default'>".$link_innerHTML."</a></div>";
    }


    // JavaScript-Link generieren:
    if($parameters["js"] != ""){
        $javascript = explode("|",$parameters["js"]);

        foreach($javascript as &$value){
            $js_html .= "<script type='text/JavaScript' src='/".$value."'></script>\n";
        }
    }


    // Stylesheet-Links generieren:
    $mathjax = ""; //MathJax
    if($parameters["css"] != ""){
        $stylesheet = explode("|",$parameters["css"]);

        foreach($stylesheet as &$value){
            if($value == "mathjax"){
                $mathjax = "
                    <script type='text/x-mathjax-config'>
                        MathJax.Hub.Config({
                        tex2jax: {inlineMath: [[\"$\",\"$\"],[\"\\\\(\",\"\\\\)\"]]}
                        });
                    </script>
                    <script type='text/javascript' async src='//cdnjs.cloudflare.com/ajax/libs/mathjax/2.7.1/MathJax.js?config=TeX-AMS_CHTML'></script>
                ";
            }
            else {
                $css_html .= "<link rel=\"stylesheet\" href=\"".$rootPath."css/".$value."\" />\n";
            }
        }
    }

    if($parameters["forwarding"] != ""){
        $forwarding_parts = explode("|",$parameters["forwarding"]);
        $forwarding_HTML = "<meta http-equiv=\"refresh\" content=\"".$forwarding_parts[1]."; URL=".$forwarding_parts[0]."\">";
    }


    // Generate HTML for menu
    $menu_html = "";
    $menu_array = array_reverse($menu_array);

    foreach($menu_array as &$menu_entry){
        if($menu_entry[2] === 1){
            if($_SERVER["REQUEST_URI"] == $menu_entry[1] OR (explode("/",$_SERVER["REQUEST_URI"])[1] == str_replace("/","",$menu_entry[1]) AND str_replace("/","",$menu_entry[1]) != "")){
                $highlighted_class = " menu_icon_current";
            }
            else {
                $highlighted_class = "";
            }
            
            $menu_html .= "
            <a href=\"".$menu_entry[1]."\" class=\"menu_link\">
                <div class=\"menu_icon".$highlighted_class."\">
                    ".$menu_entry[0]."
                </div>
            </a>
            ";
        }
    }

        


$return .= "<!DOCTYPE html>
<html lang=\"de\">
    <head>
        <meta charset=\"UTF-8\">
        <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">
        <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=yes, minimum-scale=1.0, maximum-scale=2.0\">

        <title>".$site_title."</title>


        <!-- Bootstrap -->
        <link href=\"".$rootPath."css/bootstrap/css/bootstrap.min.css\" rel=\"stylesheet\">

        <!--[if lt IE 9]>
          <script src=\"https://oss.maxcdn.com/html5shiv/3.7.2/html5shiv.min.js\"></script>
          <script src=\"https://oss.maxcdn.com/respond/1.4.2/respond.min.js\"></script>
        <![endif]-->
        <script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js\"></script>


        <link rel=\"stylesheet\" href=\"https://cdn.jsdelivr.net/npm/bootstrap-icons@1.4.1/font/bootstrap-icons.css\">
        <link rel=\"apple-touch-icon\" href=\"".$rootPath."img/logo.png\">
        <link rel=\"shortcut icon\" href=\"".$rootPath."img/logo.png\">
        <link rel=\"stylesheet\" href=\"".$rootPath."css/default.css\" />
        ".$css_html."
        <link rel=\"stylesheet\" type=\"text/css\" href=\"".$rootPath."css/gridSystem.css\">
        ".$js_html."
        ".$forwarding_HTML."
        ".$mathjax."
    </head>
    <body>
        <div class=\"all\">
            <div style='padding-bottom:5em;'>
                <div class='header'>
                    <div class='contentWidth' style='height:25px;'>
                        <img src='".$rootPath."img/logo.png' style='height:50px;width:auto;float:left;' alt=\"Logo\">
                        ".$header."
                        ".$menu_html."
                        <div style='clear:both;'></div>
                    </div>
                </div>
                <div class='contentWidth'>
                    <div class='content'>
                        ".$linkbackHTML."
                        ".$html."
                    </div>
                </div>
                <div class='footer'>
                    <div class='contentWidth'>
                        <div class='footer_inner'>
                            <div style='float:right;width:90px;display:none'>
                            <a href='".$rootPath."impressum' class='default' style='color:#000000;'>Impressum</a>
                            </div>
                            © ".date("Y").", HyQu
                        </div>
                    </div>
                </div>
            </div>
        </div>


        <script src=\"https://ajax.googleapis.com/ajax/libs/jquery/1.11.3/jquery.min.js\"></script>
        <script src=\"".$rootPath."css/bootstrap/js/bootstrap.min.js\"></script>


    </body>
</html>

";

    if($returnHTMLWithoutWhitespace === true){
        $return = str_replace("\n","",$return);
        $return = trim(preg_replace('/(\t)+/', '', $return));
        $return = trim(preg_replace('/(    )+/', '', $return));
    }

    return $return;
}
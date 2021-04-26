<?php


require_once(__DIR__.'/includes/generate_html.php');




$html = "
<h1>HyQu group website</h1>
This is the private group website of the Hybrid Quantum Systems Group. Only members of th corresponding D-PHYS group have access to this site.
The purpose of this site is to host internal interfaces to control some devices which should not be visible to the public.

<br><br><br><br>

<h2>Current services</h2>
<ul>
	<li><a href=\"lab-monitoring/\" class=\"default\">Lab monitoring</a>: Here you can reset the Arduinos for Lab monitoring and assign well-descriptive names to them.</li>
</ul>
";









// Define parameters for function that builds the HTML page and call that function
$parameters = array(
	"site_title" => "HyQU!",
	"html" => $html,
	"linkback" => "",
	"js" => "",
	"css" => "",
	"forwarding" => "",
);


echo generate_html($parameters);
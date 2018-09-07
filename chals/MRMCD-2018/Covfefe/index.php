<?php
$top = "
                        (
                          )     (
                   ___...(-------)-....___
               .-\"\"       )    (          \"\"-.
         .-'``'|-._             )         _.-|
        /  .--.|   `\"\"---...........---\"\"`   |
       /  /    |                             |
";
$middle = "       |  |    |                             |";
$bottom = "        \  \   |                             |
         `\ `\ |                             |
           `\ `|                             |
           _/ /\                             /
          (__/  \                           /
       _..---\"\"` \                         /`\"\"---.._
    .-'           \                       /          '-.
   :               `-.__             __.-'              :
   :                  ) \"\"---...---\"\" (                 :
    '._               `\"--...___...--\"`              _.'
      \\\"\"--..__                              __..--\"\"/
       '._     \"\"\"----.....______.....----\"\"\"     _.'
       jgs`\"\"--..,,_____            _____,,..--\"\"`
                        `\"\"\"----\"\"\"`";

class Covfefe {
    public $id = 42;
    public $allowed_additions = ["Milk","Cream","Whisky","Rum","Kahlua","Aquavit","Vanilla","Almond"];
    public $requested_additions = [];

    function __construct() {
        global $bottom;
        global $middle;
        $this->id = rand();
        $this->time = time();
        $this->add($bottom);
        $this->add($middle." <- Covfefe\n");
    }

    function request_ingredient($what) {
        if (in_array($what, $this->allowed_additions)) {
            $this->requested_additions[] = $what;
        }
    }

    function add_ingredient() {
        global $middle;
        $n = (time() - $this->time) / 10;
        $n = min(10, max((int)$n, 0));

        $what = $this->requested_additions[0];
        array_shift($this->requested_additions);
        if (in_array($what, $this->allowed_additions)) {
            $this->add(str_repeat($middle."\n", $n - 1));
            $this->add($middle." <- ".$what."\n");
        }
    }

    function add($line) {
        $id = max(0, $this->id);
        $fname = "covfefe/".$id;
        $content = file_get_contents($fname);
        file_put_contents($fname, $line.$content);
    }

    function get() {
        global $top;
        $id = max(0, $this->id); //implicit integer cast
        $fname = "covfefe/".$id;
        $content = file_get_contents($fname);
        unlink($fname);
        return $top . $content;
    }
}

header("HTTP/1.0 418 I'm a teapot");
$base = "http://".$_SERVER['SERVER_NAME'].$_SERVER["PHP_SELF"]."?";

if ($_SERVER["REQUEST_METHOD"] == "GET") {
    if ($_SERVER['QUERY_STRING'] == "") {
        echo "Actually I'm not a Teapot, but a RFC2324 Covfefe machine...\n";
        echo "So let me BREW something for you!\n";
        echo "You can add additions[] Milk,Cream,Whisky,Rum,Kahlua,Aquavit,Vanilla,Almond";
    } else {
        $c = unserialize(base64_decode($_SERVER['QUERY_STRING']));
        echo $c->get();
    }
}
if ($_SERVER["REQUEST_METHOD"] == "BREW") {
    $c = new Covfefe;
    if (isset($_GET['additions'])) foreach ($_GET['additions'] as $what) $c->request_ingredient($what);

    echo "Covfefe is Brewed...\n";
    $action = $base.base64_encode(serialize($c));
    if (sizeof($c->requested_additions) == 0) {
        echo "You can GET your Covfefe at $action";
    } else {
        $addition = $c->requested_additions[0];
        echo "Say WHEN you want to stop to add $addition $action";
    }
}
if ($_SERVER["REQUEST_METHOD"] == "WHEN") {
    $c = unserialize(base64_decode($_SERVER['QUERY_STRING']));
    
    $addition = $c->requested_additions[0];
    $c->add_ingredient();
    echo "Added $addition\n";
    $action = $base.base64_encode(serialize($c));
    if (sizeof($c->requested_additions) == 0) {
        echo "You can GET your Covfefe at $action";
    } else {
        $addition = $c->requested_additions[0];
        echo "Say WHEN you want to stop to add $addition $action";
    }
}

echo "\n";

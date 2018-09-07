#! /bin/sh

# always exit on error
set -e

echo "[*] setup apache2"
# remove unused configurations
rm /etc/apache2/conf.d/userdir.conf || true
rm /etc/apache2/conf.d/info.conf || true

# run-dir is missing in default installation, so we need to create it
# httpd won't start otherwise
mkdir -p /run/apache2
# make all files und sub directories in the web root read-only for the apache user 
chmod -R 644 /var/www/*
mkdir -p /var/www/covfefe
chmod -R 777 /var/www/covfefe

# test the httpd config
# if it fails, the script will fail and thus make docker build exit, too
echo "[*] testing config"
httpd -t
if [[ $? -eq 0 ]]; then
    echo "[*] config correct"
    else
        echo "[E] config not correct"
        exit 1
fi


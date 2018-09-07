# use alpine for smaller image size
FROM alpine:latest

# install all needed packets
RUN apk update && \
    apk add apache2 && \
    apk add php5-apache2

# copy all web-related stuff
ADD index.php /var/www/index.php
ADD httpd.conf /etc/apache2/httpd.conf
ADD flag.txt /flag.txt

# copy all setup scripts and helpers into the container
ADD setup_httpd.sh /root/setup/setup_httpd.sh
ADD entrypoint.sh /entrypoint.sh

# execute the setup scripts and make the entry point executable
RUN sh /root/setup/setup_httpd.sh
RUN chmod +x /entrypoint.sh

# remove all scripts/files used for setup
RUN rm -r /root/setup

# httpd is listeing on Port 8889
EXPOSE 8889

# at startup, run this script
ENTRYPOINT /entrypoint.sh


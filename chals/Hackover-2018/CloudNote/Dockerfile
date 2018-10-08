FROM ubuntu:xenial
MAINTAINER bolek42

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get dist-upgrade -y
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y socat sudo

RUN useradd -m cloudnote

ADD cloudnote /home/cloudnote/
ADD cloudnote.startup /home/cloudnote/
RUN chmod +x /home/cloudnote/cloudnote*
ADD flag /home/cloudnote/

#set up user db
RUN mkdir /home/cloudnote/db
RUN chown -R root:root /home/cloudnote/
RUN chown -R cloudnote:cloudnote /home/cloudnote/db

EXPOSE 6354
CMD cd /home/cloudnote && sudo -u cloudnote socat TCP-LISTEN:6354,fork EXEC:'/home/cloudnote/cloudnote.startup'

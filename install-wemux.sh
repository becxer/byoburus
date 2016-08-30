sudo git clone https://github.com/socc-io/wemux /usr/local/share/wemux
sudo ln -s /usr/local/share/wemux/wemux /usr/local/bin/wemux
sudo cp /usr/local/share/wemux/wemux.conf.example /usr/local/etc/wemux.conf
sudo echo "host_list=(`whoami`)" >> /usr/local/etc/wemux.conf

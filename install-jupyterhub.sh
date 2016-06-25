sudo apt-get update -y
sudo apt-get upgrade -y
sudo apt-get install python-pip -y
sudo apt-get install python3-pip -y
sudo apt-get install npm nodejs-legacy -y
sudo pip3 install jupyter
sudo npm install -g configurable-http-proxy
sudo pip3 install jupyterhub
sudo pip install ipython
sudo python -m pip install ipykernel

#  After install hub----- Do below
#  
#  jupyterhub --generate-config
#  vim jupyterhub_config.py
#  
#  # jupyterhub_config.py
#  c.JupyterHub.ip = '0.0.0.0'
#  c.JupyterHub.port = 443
#  
#  git clone https://github.com/letsencrypt/letsencrypt
#  cd letsencrypt
#  ./letsencrypt-auto --help
#  ./letsencrypt-auto certonly --standalone -d my-domain
#  
#  jupyterhub_config.py
#  c.JupyterHub.ssl_cert = '/etc/letsencrypt/live/my-domain/fullchain.pem'
#  c.JupyterHub.ssl_key = '/etc/letsencrypt/live/my-domain/privkey.pem'
#  
#  sudo jupyterhub 

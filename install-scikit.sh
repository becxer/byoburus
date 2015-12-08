
#install sci-learn
wget -O- http://neuro.debian.net/lists/trusty.jp.full | sudo tee /etc/apt/sources.list.d/neurodebian.sources.list
sudo apt-key adv --recv-keys --keyserver hkp://pgp.mit.edu:80 0xA5D32F012649A5A9
sudo apt-get update
sudo apt-get install python-sklearn
sudo update-alternatives --set libblas.so.3 /usr/lib/atlas-base/atlas/libblas.so.3
sudo update-alternatives --set liblapack.so.3 /usr/lib/atlas-base/atlas/liblapack.so.3
nosetests -v sklearn

echo "..."
echo "=================================================="
echo "Install Complete"


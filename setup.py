import os,sys
from distutils.core import setup

install_dir=os.path.join("LOLITech","matiec")

data_files=[]
os.getcwd()
os.chdir(os.getcwd())

def generate(base_dir):
    listfile=[]
    if base_dir == "":
        directory = "."
    else:
        directory = base_dir
    data_files.append((os.path.join(install_dir, base_dir), listfile))

    for element in os.listdir(directory):
        element_path=os.path.join(base_dir, element)
        if os.path.isdir(element_path):
            basename_element=os.path.basename(element_path)
            if basename_element == "lib":
                generate(element_path)
        elif os.path.isfile(element_path):
            ext_element=os.path.splitext(element)
            if ext_element[1] == ".exe" or ext_element[1] == ".h" or ext_element[1] == ".txt" or ext_element[1] == ".c":
                listfile.append(element_path)

generate("")

setup(name='MatIEC',
      version='0.1',
      description='IEC to C Compiler',
      author='Mario de Sousa, Edouard Tisserant, Laurent Bessard',
      author_email='edouard.tisserant@lolitech.fr,laurent.bessard@lolitech.fr,gregory.trelat@lolitech.fr',
      url='http://www.beremiz.org',
      license='GPL',
      data_files=data_files, # Add files to install
)
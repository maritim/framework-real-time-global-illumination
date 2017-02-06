LiteEngine
==========

Lite 3D Game Engine

<img src="https://raw.githubusercontent.com/maritim/LiteEngine/gh-pages/Particles.png" alt="Particles" width="420px"/>
<img src="https://raw.githubusercontent.com/maritim/LiteEngine/gh-pages/Animations.png" alt="Animations" width="420px"/>

Project Health
=================

| Service | System | Compiler | Status |
| ------- | ------ | -------- | ------ |
| [AppVeyor](https://ci.appveyor.com/project/maritim/liteengine)| Windows 64 | Visual Studio 2015 | [![AppVeyor](https://ci.appveyor.com/api/projects/status/s0fqli66756555gt/branch/master?svg=true)](https://ci.appveyor.com/project/maritim/liteengine/branch/master)

Debian Install Guide
=================

Get the project from GitHub
--------------------

* Install git

        sudo apt-get install -y git

* Create a local directory for the project

        mkdir LiteEngine && cd LiteEngine
    
* Clone the project

        git clone https://github.com/maritim/LiteEngine

Install and configure dependecies
--------------------

* Install dependecies

        sudo chmod +x setup.sh
        sudo ./setup.sh
    
Build
-----

* Compile the project

        make
        
* Run the application using a prototype scene

        ./GameEngine.out --startscene Assets/Scenes/Prototype.scene 

======================
Developer Guide
======================

------------------------------------------
ALCF Polaris
------------------------------------------

These are steps that are needed to compile :code:`dftracer` on `ALCF Polaris <https://docs.alcf.anl.gov/polaris/getting-started/>`_

First, make sure you have set up the environment variable and source it as shown `here <https://docs.alcf.anl.gov/polaris/data-science-workflows/python/>`_. Then, you can modify the :code:`dftracer` codebase and compile the codebase by running commands below:

.. code-block:: bash

   module use /soft/modulefiles
   module load conda
   module unload darshan
   conda activate base
   source <your venv>

   export CC=cc
   export CXX=CC
   export CMAKE_BUILD_TYPE=PROFILE
   export DFTRACER_ENABLE_TESTS=On
   export DFTRACER_LOGGER_USER=1
   export DFTRACER_DISABLE_HWLOC=On
   export DFTRACER_TEST_LD_LIBRARY_PATH=/opt/cray/libfabric/1.15.2.0/lib64
   pip install -v ".[dfanalyzer]"

.. note::

   We need to disable :code:`darshan` here because it will give you a lot of :code:`segfault` on Polaris machine due to POSIX API interceptor done by Darshan

Then, to run the the test, you need to run commands below:

.. code-block:: bash

   module use /soft/modulefiles
   module load conda
   module unload darshan
   conda activate base
   source <your venv>

   pip install -r test/py/requirements.txt
   pushd build/temp*/*pydftracer*/
   ctest -E dlio -VV --debug --stop-on-failure
   popd


Updating Docs
=============

For updating the docs we need to install additional dependency :code:`Sphinx`

.. code-block:: bash

   module use /soft/modulefiles
   module load conda
   module unload darshan
   conda activate base
   source <your venv>

   pip install "Sphinx<7"

   cd <dftracer>/docs
   make html
   
                
Then open :code:`_build/html/index.html`




Managing Configuration
==============

Importing config variables in your code is done by including the header file:

.. code-block:: cpp
   #ifdef DFTRACER_DEBUG
   #include <dftracer/dftracer_config_dbg.hpp>
   #else
   #include <dftracer/dftracer_config.hpp>
   #endif
   #ifdef DFTRACER_MY_OPTION_ENABLE 
   // Define MY_OPTION dependent code here
   #endif

Enabling configuration options is done in the :code:`cmake/dftracer_config.hpp` and
the :code:`cmake/dftracer_config_dbg.hpp` files. These files contain the definitions
of configuration options that can be used to control the behavior of the
:code:`dftracer` library. The debug version of the configuration file is used
when the :code:`DFTRACER_DEBUG` macro is defined, allowing for different
configurations in debug builds compared to release builds.

.. code-block:: cpp
   #cmakedefine DFTRACER_MY_OPTION_ENABLE 1

Defining the configuration options is done in the :code:`CMakeLists.txt` file

.. code-block:: cmake
   option(DFTRACER_ENABLE_MY_OPTION "Enable MY_OPTION" ON)
   if (DFTRACER_ENABLE_MY_OPTION)
       set(DFTRACER_MY_OPTION_ENABLE 1)
   endif()

The code can build with the configuration options by setting the
:code:`DFTRACER_ENABLE_MY_OPTION` option in the CMake configuration step

.. code-block:: bash
   cmake -DDFTRACER_ENABLE_MY_OPTION=ON <other options> <path to dftracer source>
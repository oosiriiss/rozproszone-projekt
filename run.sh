#!/bin/bash


clean() {
   echo "Cleaning..."
   rm -r build bin &> /dev/null
}


if [[ $# == 0 ]]; then
   clean

   echo "Generating build files"

   mkdir build
   cd build

   generation_result= cmake .. -G "Ninja"

   if [[ generation_result -eq 0 ]]; then
      echo "Compiling..."

      # Compilation
      compilation_result= ninja

      if [[ compilation_result -eq 0 ]]; then 
	 echo "running executable"
	 echo ""
	 cd ../bin
      	 ./executable
      else 
	 echo "ninja build failed"
	 echo "ninja exit code " + $compilation_result
      fi


   else
      echo "cmake failed when generating ninja files"
      echo "cmake exit code " + $generation_result
   fi

elif [[ "$1" == "clean" ]]; then
   clean

else
   echo "Unknown option"
fi



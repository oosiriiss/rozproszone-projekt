#!/bin/bash


clean() {
   echo "Cleaning..."
   rm -r build bin &> /dev/null
}


compile() {
   echo "Generating build files"

   mkdir build
   cd build

   generation_result= cmake .. -G "Ninja"

   if [[ generation_result -eq 0 ]]; then
      echo "Compiling..."

      # Compilation
      compilation_result= ninja

      if [[ compilation_result -ne 0 ]]; then 
	 echo "ninja build failed"
	 echo "ninja exit code " + $compilation_result

      fi
      return 0
   else
      echo "cmake failed when generating ninja files"
      echo "cmake exit code " + $generation_result

      return generation_result
   fi


   return 0
}


if [[ $# == 0 ]]; then
   clean

   compile_result= compile

   if [[ compile_result -eq 0 ]]; then
	 echo "running executable"
	 echo ""
	 cd ../bin
      	 ./executable
   else 
      echo "Couldn't run exectuable"
   fi
elif [[ "$1" == "compile" ]]; then

   clean 
   compile

elif [[ "$1" == "clean" ]]; then
   clean

else
   echo "Unknown option"
fi



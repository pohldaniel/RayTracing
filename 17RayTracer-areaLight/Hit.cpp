#include <iostream>
#include "Hit.h"
#include "Scene.h"





Hit::Hit(){

	t = FLT_MAX;
	color = Color(0, 0, 0);
	hitObject = false;

}

Hit::~Hit(){


}
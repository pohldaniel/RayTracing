#include <iostream>
#include "Hit.h"
#include "Scene.h"

Hit::Hit(Scene *a_scene){

	t = FLT_MAX;
	color = Color(0, 0, 0);
	hitObject = false;
	m_scene = std::shared_ptr<Scene>(a_scene);
	
}



Hit::Hit(){

	t = FLT_MAX;
	color = Color(0, 0, 0);
	hitObject = false;

}

Hit::~Hit(){


}
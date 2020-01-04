//#define INDIRECTLIGHT 1
//#define DIRECTLIGHT 1
//#define DEBUG_PT 1
#include <string>
#include <iostream>
#include <fstream>
#include <math/Vect3f.h>
#include <math/Ray.h>
#include <world/Display.h>
#include <world/objects/Object.h>
#include "PT.h"
#include "ioUtils.h"
#include <math/MatUtils.h>
#include <random/RandUtils.h>

#include<random>
#include<cmath>
#include<chrono>


Vect3f getPoint(const RandUtils &r) {
    auto phi = static_cast<float>(2 * M_PI * r.nextFloat(0.0f,1.0f));
    float theta = acos(sqrt(1 - r.nextFloat(0.0f,1.0f)));
    float x = sin(theta) * cos(phi);
    float y = cos(theta);
    float z = sin(theta) * sin(phi);
    return Vect3f(x, y, z);
}

Vect3f generateDirectionDiffuse(const RandUtils &distA, const RandUtils &distB, Vect3f& normal, Vect3f& collisionPoint, Vect3f& direccion){
    float randA = distA.nextFloat(0.0f,1.0f);
    float randB = distB.nextFloat(0.0f,1.0f);
    auto phi = static_cast<float>(2 * M_PI * randA);
    float theta = acos(sqrt(1 - randB));
    float x = sin(theta) * cos(phi);
    float y = cos(theta);
    float z = sin(theta) * sin(phi);
    Vect3f ansBaseAntigua(x, y, z);

    Vect3f aux = (normal ^ (direccion.neg())).normalized();

    Vect3f aux2 = (normal ^ aux).normalized();

    Mat4f cambioDeBase = Mat4f(aux, normal, aux2, Vect3f(0,0,0));
    Vect3f ans = cambioDeBase.inverse() * ansBaseAntigua;

    return ans.neg();
}

Vect3f generateDirectionSpecular(const RandUtils &distA, const RandUtils &distB, Vect3f& normal, Vect3f& collisionPoint, Vect3f& direccion, float shiness){
    float randA = distA.nextFloat(0.0f,1.0f);
    float randB = distB.nextFloat(0.0f,1.0f);
    auto phi = static_cast<float>(2.0f * M_PI * randA);
    float theta = std::acos(std::pow(randB, 1 / (shiness + 1)));
    float x = sin(theta) * cos(phi);
    float y = cos(theta);
    float z = sin(theta) * sin(phi);
    Vect3f ansBaseAntigua(x, y, z);

    Vect3f aux = (normal ^ (direccion.neg())).normalized();

    Vect3f aux2 = (normal ^ aux).normalized();

    Mat4f cambioDeBase = Mat4f(aux, normal, aux2, Vect3f(0,0,0));
    Vect3f ans = cambioDeBase.inverse() * ansBaseAntigua;

    return ans.neg();
}

Color PT::TracePath(Ray ray) {

    bool stop = false;
    Object *collidedObject;



    Vect3f direction = ray.getDirection(), currentPosition;
    Vect3f collisionPoint = ray.getOrigin();

    Vect3f pointOrigen = collisionPoint;
    // Bloque es un espejo o cristal
    int mirrorCollisions = 0;// corte ingles
    do{
        // get origen del rayo del primer rebote y el objeto en el que se produce
        std::tie(collisionPoint, collidedObject, std::ignore) = w.getNearestCollision(collisionPoint, direction, false);
        if (collidedObject != nullptr && collidedObject->getMaterialType() != MaterialType::DIFFUSE){
            std::tie(direction, std::ignore, collisionPoint) = w.getOutputDirection(collidedObject,collisionPoint, direction);
        }
        mirrorCollisions++;
    }while((collidedObject != nullptr && (collidedObject->getMaterialType() != MaterialType::DIFFUSE)) && mirrorCollisions < MAX_MIRROR_REFLECTIONS);
#ifdef DEBUG_PT
    if( collidedObject == nullptr ){
        std::cout << "casa 0" << endl;
    }
    else if(collidedObject->getMaterialType() == MaterialType::DIFFUSE){
//        std::cout << "casa 1" << endl;
    }
    else if (mirrorCollisions > MAX_MIRROR_REFLECTIONS ){
        std::cout << "casa 2" << endl;
    }
    else{
        std::cout << "raro" << endl;
    }
#endif
    // FIN Bloque es un espejo o cristal

    // si no colisiona con nada color negro
    if (collidedObject == nullptr) {
        return Color(0.01f, 0.01f, 0.01f);  // Nothing was hit.
    }

    // Si se colisiona con una luz color blanco
    if (collidedObject->getPower() > 0){
        return Color(.9295f, .9355f, .929f);
    }
#ifdef DIRECTLIGHT

    Color directLight = w.calculateDirectLight(collisionPoint,*collidedObject, direction);


#else

    //    Color directLight = w.calculateDirectLight(collisionPoint,*collidedObject);
    int k = 0;
    RussianRoulette rr;
    NextEventBRDF nextEvent ;

    Vect3f productorioBRDF(1.0,1.0,1.0);
    Color sumatorioLuz = Color(0, 0, 0);
    Color ultimoRebote = Color(0, 0, 0);

    while(!stop && k < 10  ){

        do{
            // get origen del rayo del primer rebote y el objeto en el que se produce
            std::tie(collisionPoint, collidedObject, std::ignore) = w.getNearestCollision(collisionPoint, direction, false);
            if (collidedObject != nullptr && collidedObject->getMaterialType() != MaterialType::DIFFUSE){
                std::tie(direction, std::ignore, collisionPoint) = w.getOutputDirection(collidedObject,collisionPoint, direction);
            }
            mirrorCollisions++;
        }while((collidedObject != nullptr && (collidedObject->getMaterialType() != MaterialType::DIFFUSE)) && mirrorCollisions < MAX_MIRROR_REFLECTIONS);
#ifdef DEBUG_PT
        if( collidedObject == nullptr ){
            std::cout << "casa 0" << endl;
        }
        else if(collidedObject->getMaterialType() == MaterialType::DIFFUSE){
//        std::cout << "casa 1" << endl;
        }
        else if (mirrorCollisions > MAX_MIRROR_REFLECTIONS ){
            std::cout << "casa 2" << endl;
        }
        else{
            std::cout << "raro" << endl;
        }
#endif

        if(collidedObject != nullptr){ // Si colisiona
            if(collidedObject->getPower() != 0){ // Si es una luz

                Color colorLuz = collidedObject -> getTexture()->getColorAt(collisionPoint);
                ultimoRebote = collidedObject->getPower() * colorLuz;

                stop = true;

            }
            else{ // Si es un objeto
//                if(k != 0)
                sumatorioLuz = sumatorioLuz + (productorioBRDF & w.calculateDirectLight(collisionPoint,*collidedObject, direction));

                nextEvent = rr.getNextEventBRDF(collidedObject->getKD(),collidedObject->getKS());

                // Se calcula direccion de salida del rebote
                if(nextEvent == NextEventBRDF::LAMBERTIAN){//rebote difuso
                    Vect3f normal = collidedObject->getNormalAt(collisionPoint);

                    // comprueba si la normal es correcta
                    if (normal.angle(direction.neg()) > M_PI_2 ){
                        normal = normal.neg();
                    }

                    Vect3f newDirection =  generateDirectionDiffuse(distA,distB, normal, collisionPoint, direction);
                    if (normal.angle(newDirection)>M_PI_2 ){
                        std::cout << "fallo 3" << endl;
                        newDirection = newDirection.neg();
                    }

                    //Difusa
                    productorioBRDF = productorioBRDF & ((brdf.calculateLambertian(*collidedObject, collisionPoint)  * std::abs(normal * newDirection))  / ( 0.9f * collidedObject->getKD()));
#ifdef DEBUG_PT
                    if (productorioBRDF.getX() < 0 || productorioBRDF.getY() < 0 || productorioBRDF.getZ() < 0 ){
                        std::cout << "error 0" << endl;
                    }
                    if (normal.angle(direction.neg()) > M_PI_2 ){
                        std::cout << "error n" << endl;
                    }
#endif
                    direction = newDirection;
                }
                else if(nextEvent == NextEventBRDF::SPECULAR ){
                    Vect3f normal =  collidedObject->getNormalAt(collisionPoint);
                    if (normal.angle(direction.neg()) > M_PI_2 ){
                        normal = normal.neg();
                    }

                    //Monte Carlo
                    Vect3f newDirection = generateDirectionSpecular(distA, distB, normal, collisionPoint, direction, collidedObject->getShininess());
                    if (normal.angle(newDirection)>M_PI_2 ){
                        std::cout << "fallo" << endl;
                        newDirection = newDirection.neg();
                    }

                    float theta = normal.angle(direction);
                    Vect3f localBRDF = brdf.calculateSpecular(*collidedObject, collisionPoint, normal, newDirection, direction.neg());

                    productorioBRDF = productorioBRDF & ( localBRDF / (0.9f * collidedObject->getKS()));
//                    productorioBRDF = productorioBRDF & (M_2_PI * localBRDF & (Vect3f(1,1,1)* /* path tracing pg 35*/std::abs(normal * newDirection)/ (0.9f * collidedObject->getKS()* (collidedObject->getShininess() + 1) * sin(theta) * pow(cos(theta),collidedObject->getShininess()))));
#ifdef DEBUG_PT
                    if (productorioBRDF.getX() < 0 || productorioBRDF.getY() < 0 || productorioBRDF.getZ() < 0 ){
                        std::cout << "error 0" << endl;
                    }
                    if (normal.angle(direction.neg())> M_PI_2 ){
                        std::cout << "error n" << endl; //limpio
                    }
#endif
                    //Especular con Phong
                    // lo que multiplica al productorio es brdf * cos ( simbolo raro xd) // probabilidad
                    direction = newDirection;

                }

                else{
                    stop = true;
                }
            }

        }
        else{
            stop = true;
        }
        k++;
    }
#endif
    //Apply the Rendering Equation here.
#ifdef DIRECTLIGHT
    return directLight;
#elif INDIRECTLIGHT
    return sumatorioLuz + (productorioBRDF & ultimoRebote) - directLight;
#else
    return sumatorioLuz ;// (productorioBRDF & ultimoRebote);
#endif
}


void PT::Render( int  PathXPixel, std::string &fileName) {

    unsigned int pixels = display.getWidth() * display.getHeight();
    std::vector<Vect3f> image(pixels);
    unsigned i = 0;
    float percentage = 0.0f;


    for (int y = 0; y < display.getHeight(); ++y) {
        for (int x = 0; x < display.getWidth(); ++x) {
            for (int j = 0; j < PathXPixel; ++j) { // Antialiasing
//                if (x == 411 && y == 178)
//                    std::cout << "eureka" << std::endl;
//                if (x == 243 && y == 403)
//                    std::cout << "eureka" << std::endl;
                float dx = w.getRandom().nextFloat();
                float dy = w.getRandom().nextFloat();
//                Vect3f v = w.getCamera().getDirection(this->display.convertPixels(x + dx, y + dy));
//                Vect3f direction = (v).normalized();
//
//                Ray r = Ray(w.getCamera().getPosition(), direction);
                Ray r = w.getCamera().getRay(this->display.convertPixels(x + dx, y + dy));
                image[i] = image[i] + TracePath(r);
            }
            image[i] = image[i] * 255 / static_cast<float>(PathXPixel);  // Average samples.

            ++i;
            float newPercentage = 100.0f * i / pixels;
            if((unsigned)percentage != (unsigned)newPercentage){
                std::cout << "\rPercentage: " << newPercentage << "%" << std::flush;
            }
            percentage = newPercentage;
        }
    }
    savePT(fileName, image, display.getWidth(), display.getHeight());// Antialisasing
}
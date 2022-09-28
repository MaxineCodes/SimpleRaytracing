#include "rtweekend.h"

#include "colour.h"
#include "hittable_list.h"
#include "sphere.h"
#include "camera.h"
#include "material.h"

#include <iostream>



double hit_sphere(const point3& center, double radius, const ray& r)
{
    vec3 oc = r.origin() - center;
    auto a = r.direction().length_squared();
    auto half_b = dot(oc, r.direction());
    auto c = oc.length_squared() - radius * radius;
    auto discriminant = half_b * half_b - a * c;

    if (discriminant < 0) 
    {
        return -1.0;
    }
    else 
    {
        return (-half_b - sqrt(discriminant)) / a;
    }
}


color ray_color(const ray& r, const hittable& world, int depth)
{
    hit_record rec;

    // If we've exceeded the ray bounce limit, no more light is gathered.
    if (depth <= 0) 
    {
        return color(0, 0, 0);
    }

    if (world.hit(r, 0.001, infinity, rec))
    {
        ray scattered;
        color attenuation;
        if (rec.mat_ptr->scatter(r, rec, attenuation, scattered))
            return attenuation * ray_color(scattered, world, depth - 1);
        return color(0, 0, 0);
    }

    vec3 unit_direction = unit_vector(r.direction());
    auto t = 0.5 * (unit_direction.y() + 1.0);
    return (1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0);
}

int main() 
{

    // Image
    const auto aspect_ratio = 16.0 / 9.0;
    const int image_width = 800;
    const int image_height = static_cast<int>(image_width / aspect_ratio);
    const int samples_per_pixel = 250;
    const int max_depth = 10;


    // World
    auto R = cos(pi / 4);
    hittable_list world;

    // Materials
    auto material_ground        = make_shared<lambertian>   (color(0.3, 0.0, 0.4));
    auto material_lambertian    = make_shared<lambertian>   (color(0.9, 0.1, 0.6));
    auto material_dialectric    = make_shared<dielectric>   (1.5);
    auto material_metal         = make_shared<metal>        (color(0.7, 0.7, 0.7), 0.2);
    auto material_fuzzyMetal    = make_shared<metal>        (color(0.7, 0.7, 0.7), 0.9);
    auto material_bronze        = make_shared<metal>        (color(0.8, 0.45, 0.3), 0.6);
    auto material_redMetal      = make_shared<metal>        (color(1.0, 0.0, 0.0), 0.1);

    // Sphere objects
    // Ground sphere
    world.add(make_shared<sphere>(point3(0.0, -1000.5, -1.0), 1000.0, material_ground));
    // Middle sphere, lambert
    world.add(make_shared<sphere>(point3(0.0, 0.0, -1.0), 0.5, material_lambertian));
    // Left sphere, dialectric (2 objects: hollow doublesided sphere)
    world.add(make_shared<sphere>(point3(-1.0, 0.0, -1.0), 0.5, material_dialectric));
    world.add(make_shared<sphere>(point3(-1.0, 0.0, -1.0), -0.49, material_dialectric));
    // Sphere inside left sphere, metal
    world.add(make_shared<sphere>(point3(-1.0, 0.0, -1.0), 0.2, material_bronze));
    // Right sphere, metal
    world.add(make_shared<sphere>(point3(1.0, 0.0, -1.0), 0.5, material_metal));
    // Small left sphere, full dialectric
    world.add(make_shared<sphere>(point3(-1.2, -0.3, -0.3), 0.2, material_dialectric));
    // Small left sphere, hollow dialectric
    world.add(make_shared<sphere>(point3(-0.6, -0.3, -0.3), 0.2, material_dialectric));
    world.add(make_shared<sphere>(point3(-0.6, -0.3, -0.3), -0.19, material_dialectric));
    // Small middle sphere, metal
    world.add(make_shared<sphere>(point3(0.0, -0.3, -0.3), 0.2, material_fuzzyMetal));
    // Small right sphere, metal
    world.add(make_shared<sphere>(point3(0.6, -0.3, -0.3), 0.2, material_redMetal));
    // Small right sphere, metal
    world.add(make_shared<sphere>(point3(1.2, -0.3, -0.3), 0.2, material_bronze));


    // Camera
    point3 lookfrom(0.35, 0.5, 2);
    point3 lookat(0, 0, -0.75);
    vec3 vup(0, 1.75, 0);
    auto dist_to_focus = (lookfrom - lookat).length();
    auto aperture = 0.075;

    // Camera position(worldspace), target(worldspace), FOV, aspectratio(from main) aperture
    camera cam(lookfrom, lookat, vup, 40, aspect_ratio, aperture, dist_to_focus);


    // Render

    std::cout << "P3\n" << image_width << " " << image_height << "\n255\n";

    for (int j = image_height - 1; j >= 0; --j) 
    {
        std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
        for (int i = 0; i < image_width; ++i) 
        {
            color pixel_color(0, 0, 0);
            for (int s = 0; s < samples_per_pixel; ++s) 
            {
                auto u = (i + random_double()) / (image_width - 1);
                auto v = (j + random_double()) / (image_height - 1);
                ray r = cam.get_ray(u, v);
                pixel_color += ray_color(r, world, max_depth);
            }
            write_colour(std::cout, pixel_color, samples_per_pixel);
        }
    }

    std::cerr << "\nDone.\n";
}
PImage eye_img;
int iteration = 0;

void setup() {
    size(640, 360, P3D);
    fill(204);
    eye_img = loadImage("eye.png");
}


void draw() {
    lights();
    background(255);

// Handle where the little character will be
    float me_v = radians(iteration++);
    float me_x = sin(me_v)   * 400;
    float me_y = cos(me_v)   * 400;
    float me_z = cos(me_v*2) * 400;

    float z_location;
    if ( mousePressed ) { 
        z_location = float(-400); 
        camera(
            (mouseX-width/2)*10+me_x, (mouseY-height/2)*10+me_y, me_z + 500, 
            me_x, me_y, me_z, 
            0, 1, 0
        );
    } 
    else { 
        z_location = float(-4000); 
        camera(
            (mouseX-width/2)*10, (mouseY-height/2)*10, z_location, 
            0, 0, 0,
            0, 1, 0
        );

    };

// Create the suspension system
    noStroke();
    pushMatrix();
        translate(0,-1000,0);
        pushMatrix(); translate(1000,0,1000); box(50); popMatrix();
        pushMatrix(); translate(-1000,0,1000); box(50); popMatrix();
        pushMatrix(); translate(1000,0,-1000); box(50); popMatrix();
        pushMatrix(); translate(-1000,0,-1000); box(50); popMatrix();
    popMatrix();

// Draw the four suspension wires
    stroke(0);
    line(me_x,me_y-100,me_z,1000,-1000,1000);
    line(me_x,me_y-100,me_z,-1000,-1000,1000);
    line(me_x,me_y-100,me_z,1000,-1000,-1000);
    line(me_x,me_y-100,me_z,-1000,-1000,-1000);

// Create the main body
    noStroke();
    color cl = color(255, 255, 255,254);
    fill(cl);
    pushMatrix();
    translate(me_x,me_y,me_z);
    sphere(100);

// Create the arms
        noStroke();
        color arms = color(255,255,255);
        fill(arms);
        pushMatrix();
        translate(50,40,80);
        sphere(20);
        popMatrix();

        pushMatrix();
        translate(-50,40,80);
        sphere(20);
        popMatrix();

// Create the eyes
        pushMatrix();
            translate(-20,-40,75);
            beginShape();
                texture(eye_img);
                vertex(-15, -15, 0, 0, 0);
                vertex( 15, -15, 0, 342, 0);
                vertex( 15,  15, 0, 342, 342);
                vertex(-15,  15, 0, 0, 342);
            endShape();
        popMatrix();
        pushMatrix();
            translate(20,-40,75);
            beginShape();
                texture(eye_img);
                vertex(-15, -15, 0, 0, 0);
                vertex( 15, -15, 0, 342, 0);
                vertex( 15,  15, 0, 342, 342);
                vertex(-15,  15, 0, 0, 342);
            endShape();
        popMatrix();

    popMatrix();

}


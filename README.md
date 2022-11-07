# 2022GPA

* #### Environment
    * OS: macOS Big Sur 11.5.2
    * Xcode: 13.2.1
* #### Main idea 
    * ![](https://i.imgur.com/FctmDSy.png)

    * Robot:
        
        *    A vector of Bone all the parts of the robot, including torso, head, upper hands, lower hands, upper legs and lower leg.
        *    They have unique IDs:
        *    Torso ->0
        *    Head  ->1
        *    Upper right hand ->2
        *    Lower right hand ->3            
        *    Upper left hand ->4
        *    Lower left hand ->5
        *    Upper right leg  ->6
        *    Lower right leg  ->7
        *    Upper left leg  ->8
        *    Lower left leg  ->9      
    * Bone:
        * Is the basic element of the robot
        * It stores one part's position, rotation, texture, vao and vbo.
        ![](https://i.imgur.com/n4a0Tgh.png)

    * body_trans:
        * They have unique IDs
            * torso to head ->0
            * upper hand to lower hand ->1
            * torso to leg ->2
            * upper leg to lower leg ->3
    * body_rotates:
        * They have unique IDs
            * upper hand to lower hand ->0
            * torso to upper leg ->1

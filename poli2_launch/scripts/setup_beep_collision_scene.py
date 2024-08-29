#!/usr/bin/env python

# Copyright (c) 2017, Elaine Short, SIM Lab
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# 
# * Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
# 
# * Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
# 
# * Neither the name of the SIM Lab nor the names of its
#   contributors may be used to endorse or promote products derived from
#   this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import rospy
from planning_scene_helper import PlanningSceneHelper
from tf.transformations import quaternion_from_euler

def m(inch):
    """meters from inches"""
    conv = 0.0254
    if type(inch) in (tuple, list) :
        return [x*conv for x in inch]
    return inch*conv

if __name__=="__main__":
    rospy.init_node("planning_scene_setup")
    p = PlanningSceneHelper()

    p.remove("table")
    p.remove("back")
    p.remove("roof")
    p.remove("head")

    table_z = -0.04
    table_thickness = 0.05
    height = m(27.75)
    post_xsize = m(3)
    post_ysize = m(3)
    foot_ysize = m(15)
    foot_xsize = m(3)
    foot_zsize = m(3)
    # make it much taller than strictly necessary just so we never pass in front of and above the crossbar
    top_bar_thickness = 0.3
    bar_pos = m(-22)

    safety_buffer = 0.04



    p.add_box("back", "base_link", size=(0.1, 2, 2), position=(-0.35, 0.0, 1), color=(0,0,1.,0.4))
    p.add_box("head", "base_link", size=(0.28, 0.23,0.2), position=(0.0, 0.06, 1.22), color=(0.7,0.0,1.0,0.4))
    p.add_box("table", "base_link", size=(0.6, 2, table_thickness), position=(0.9144, 0.0, .73), color=(0,0,1.0,0.4))

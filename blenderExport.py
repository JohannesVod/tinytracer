import bpy

# Specify the file path where you want to save the .obj content
output_file_path = r"\\wsl.localhost\Arch\home\NiceGuySaysHi\projects\raytracer\fancytracer\baseScene.obj"

# Ensure you are in 'OBJECT' mode
if bpy.context.object.mode != 'OBJECT':
    bpy.ops.object.mode_set(mode='OBJECT')

# Get the active object
obj = bpy.context.object

# Ensure we have a mesh object selected
if obj and obj.type == 'MESH':
    # Duplicate the object to work on it non-destructively
    temp_obj = obj.copy()
    temp_obj.data = obj.data.copy()
    bpy.context.collection.objects.link(temp_obj)
    
    # Make the duplicated object the active object
    bpy.context.view_layer.objects.active = temp_obj

    # Apply all modifiers on the duplicated object
    for modifier in temp_obj.modifiers:
        bpy.ops.object.modifier_apply(modifier=modifier.name)

    # Enter edit mode and triangulate the mesh
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.mesh.select_all(action='SELECT')
    bpy.ops.mesh.quads_convert_to_tris()
    bpy.ops.object.mode_set(mode='OBJECT')
    
    # Get the mesh data of the duplicated object
    temp_mesh = temp_obj.data

    # Open the specified file for writing
    with open(output_file_path, 'w') as file:
        # Write the header information
        file.write("# Blender 4.1.1\n")
        file.write("# www.blender.org\n")
        file.write(f"o {obj.name}\n")
        
        # Write the coordinates of all vertices in the mesh
        for v in temp_mesh.vertices:
            co = v.co
            file.write(f"v {co.x:.6f} {co.y:.6f} {co.z:.6f}\n")
        
        # Write the normals of all vertices in the mesh
        for v in temp_mesh.vertices:
            normal = v.normal
            file.write(f"vn {normal.x:.6f} {normal.y:.6f} {normal.z:.6f}\n")
        
        # Write the UV coordinates of all vertices in the mesh
        uv_layer = temp_mesh.uv_layers.active.data if temp_mesh.uv_layers.active else None
        if uv_layer:
            uv_dict = {loop.vertex_index: uv_layer[loop.index].uv for loop in temp_mesh.loops}
            for v in temp_mesh.vertices:
                uv = uv_dict.get(v.index)
                if uv:
                    file.write(f"vt {uv.x:.6f} {uv.y:.6f}\n")
                else:
                    file.write(f"vt 0.000000 0.000000\n")  # Default UV coordinates if none found
        
        # Write the faces (triangles) in the mesh
        for poly in temp_mesh.polygons:
            face_str = "f"
            for loop_index in poly.loop_indices:
                loop = temp_mesh.loops[loop_index]
                vertex_index = loop.vertex_index + 1  # OBJ format indices are 1-based
                uv_index = loop_index + 1 if uv_layer else 0  # Same for texture indices
                normal_index = vertex_index  # Assume normal indices match vertex indices
                if uv_layer:
                    face_str += f" {vertex_index}/{uv_index}/{normal_index}"
                else:
                    face_str += f" {vertex_index}//{normal_index}"
            file.write(face_str + "\n")
    
    # Remove the temporary object from the scene
    bpy.data.objects.remove(temp_obj)

else:
    print("No mesh object selected.")

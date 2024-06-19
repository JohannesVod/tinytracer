import bpy

# Specify the file path where you want to save the .obj content
output_file_path = r"\\wsl.localhost\Arch\home\NiceGuySaysHi\projects\raytracer\fancytracer\baseScene.obj"

material_names_dict = {}

# Iterate over all objects in the scene
for obj in bpy.context.scene.objects:
    # List to store material names for the current object
    obj_material_names = []
    
    if obj.type == 'MESH':  # Ensure the object is a mesh
        for mat_slot in obj.material_slots:
            if mat_slot.material:  # Check if the material slot has a material
                obj_material_names.append(mat_slot.material.name)
    
    # Add the object's material names to the dictionary
    material_names_dict[obj.name] = obj_material_names

# Export the selected objects to an OBJ file using the new operator for Blender 4.0 and later
bpy.ops.wm.obj_export(filepath=output_file_path,
                      export_materials=False,
                      forward_axis='Y',  # Forward axis
                      export_triangulated_mesh=True,
                      up_axis='Z')

# Read the file and write every line again manually, overwriting the same file
with open(output_file_path, 'r') as input_file:
    lines = input_file.readlines()

# Open the file in write mode to clear its contents and then write the lines back
curr_ob = None
curr_mat = None
with open(output_file_path, 'w') as output_file:
    for line in lines:
        if line[0] == 'o':
            curr_ob = line[2:-1]
            curr_mat = material_names_dict[curr_ob][0]
        if line[:2] == "f ":
            output_file.write(line[:-1] + " ")
            output_file.write(str(curr_mat) + '\n')
        else:
            output_file.write(line)

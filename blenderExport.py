import bpy

# Specify the file path where you want to save the .obj content
output_file_path = r"\\wsl.localhost\Arch\home\NiceGuySaysHi\projects\raytracer\fancytracer\baseScene.obj"

# Export the selected objects to an OBJ file using the new operator for Blender 4.0 and later
bpy.ops.wm.obj_export(filepath=output_file_path,
                        export_materials=False,
                        forward_axis='Y',  # Forward axis
                        up_axis='Z')

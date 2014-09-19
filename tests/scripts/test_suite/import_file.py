import ovito
from ovito.io import import_file

test_data_dir = "../../files/"

node1 = import_file(test_data_dir + "animation.dump.gz")
assert(ovito.dataset.selected_node == node1)
assert(ovito.dataset.scene_nodes[0] == node1)

node2 = import_file(test_data_dir + "fcc_coherent_twin.0.cfg")
assert(len(ovito.dataset.scene_nodes) == 2)

node3 = import_file(test_data_dir + "movie.0000000.parcas")
assert(len(ovito.dataset.scene_nodes) == 3)

node4 = import_file(test_data_dir + "shear.void.120.cfg")
node5 = import_file(test_data_dir + "nw2.imd.gz")

node1.remove_from_scene()
node2.remove_from_scene()
node3.remove_from_scene()
node4.remove_from_scene()
node5.remove_from_scene()

node = import_file(test_data_dir + "multi_sequence_1.dump")
assert(ovito.dataset.anim.last_frame == 2)
node.remove_from_scene()
node = import_file(test_data_dir + "shear.void.dump.bin", 
                            columns = ["Particle Identifier", "Particle Type", "Position.X", "Position.Y", "Position.Z"])
node.remove_from_scene()
try:
    # This should generate an error:
    node = import_file(test_data_dir + "shear.void.dump.bin",  
                                columns = ["Particle Identifier", "Particle Type", "Position.X", "Position.Y", "Position.Z", "ExtraProperty"])
    assert False
except RuntimeError:
    pass

node = import_file(test_data_dir + "animation1.dump")
assert(ovito.dataset.anim.last_frame == 0)
node.remove_from_scene()

node = import_file(test_data_dir + "animation1.dump", multiple_frames = True)
assert(ovito.dataset.anim.last_frame == 10)

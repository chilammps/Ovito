import ovito
import ovito.io

test_data_dir = "../../files/"

node = ovito.io.import_file(test_data_dir + "animation.dump.gz")
assert(ovito.dataset.selected_node == node)
assert(ovito.dataset.scene_nodes[0] == node)

node2 = ovito.io.import_file(test_data_dir + "fcc_coherent_twin.0.cfg")
assert(len(ovito.dataset.scene_nodes) == 2)

node3 = ovito.io.import_file(test_data_dir + "movie.0000000.parcas", mode = "ReplaceSelected")
assert(len(ovito.dataset.scene_nodes) == 2)
assert(node3 == node2)

node4 = ovito.io.import_file(test_data_dir + "shear.void.120.cfg", mode = "ResetScene")
assert(len(ovito.dataset.scene_nodes) == 1)

ovito.io.import_file(test_data_dir + "nw2.imd.gz")
node = ovito.io.import_file(test_data_dir + "multi_sequence_1.dump", mode = "ResetScene")
node.wait()
assert(ovito.dataset.anim.last_frame == 2)

node = ovito.io.import_file(test_data_dir + "shear.void.dump.bin", mode = "ResetScene", 
                            columns = ["Particle Identifier", "Particle Type", "Position.X", "Position.Y", "Position.Z"])

try:
    # This should generate an error:
    node = ovito.io.import_file(test_data_dir + "shear.void.dump.bin", mode = "ResetScene", 
                                columns = ["Particle Identifier", "Particle Type", "Position.X", "Position.Y", "Position.Z", "ExtraProperty"])
    assert False
except RuntimeError:
    pass

node = ovito.io.import_file(test_data_dir + "animation1.dump", mode = "ResetScene")
assert(ovito.dataset.anim.last_frame == 0)

node = ovito.io.import_file(test_data_dir + "animation1.dump", mode = "ResetScene", multiple_frames = True)
assert(ovito.dataset.anim.last_frame == 10)

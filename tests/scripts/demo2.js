// Set the working directory and import a data file.
cd("../files");
node = load("shear.void.dump.bin", { 
	isMultiTimestepFile: true,
	columnMapping : ["Particle Identifier", "Particle Type", "Position.X", "Position.Y", "Position.Z"] 
});

wait();

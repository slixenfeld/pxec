use std::env;
use std::fs::read_to_string;
use std::fs::File;
use std::fs;
use std::io::{self, BufRead};
use std::io::prelude::*;
use std::path::Path;
use std::io::{BufWriter, Write};
use rand::{distributions::Alphanumeric, Rng};
use std::process::Command;

struct MapEntry {
	name: String,
	category: String,
	filehash: String
}

fn help() {
	println!("pxc commands:");
	println!("  ls (list)");
	println!("  a (add)");
	println!("  e (edit)");
	println!("  r (remove)");
	println!("  init (create map.pxc)");
	println!("  pkg");
	println!("    install <pkg>");
	println!("    remove <pkg>");
}

fn gen_char_sequence() -> String {

    const CHARSET: &[u8] = b"ABCDEF0123456789";
    const PASSWORD_LEN: usize = 8;
    let mut rng = rand::thread_rng();

    let password: String = (0..PASSWORD_LEN)
        .map(|_| {
            let idx = rng.gen_range(0..CHARSET.len());
            CHARSET[idx] as char
        })
        .collect();

    println!("{:?}", password);

	return password;
}

fn check_sequence_exists(sequence: &str, entries: &mut Vec<MapEntry>) -> bool {
	for entry in entries {
		if sequence == entry.filehash {
			return true;
		}
	}
	return false;
}

fn main() {

	let mut entries: Vec<MapEntry> = read_map_file("pxc");
	let mut args = env::args().skip(1);

	if let Some(arg) = args.next() {
		match &arg[..] {
			"h" => help(),
			"a" | "add" => {

				let mut entry_name = "".to_owned();
				let mut entry_category= "".to_owned();

				if let Some(arg1) = args.next() {
					entry_name.push_str(&arg1);
				} else {
					println!("[add] no name supplied, exiting.");
					return;
				}
				if let Some(arg1) = args.next() {
					entry_category.push_str(&arg1);
				} else {
					println!("[add] adding {} with default category", entry_name);
				}
					
				let mut char_sequence = gen_char_sequence();
				while check_sequence_exists(&char_sequence, &mut entries) {
					println!("filehash {} already existed!, generating again..", &char_sequence);
					char_sequence = gen_char_sequence();
				}

					add( MapEntry {name: entry_name.to_string(),category: entry_category.to_string(),filehash: char_sequence}, &mut entries);
					edit(&entry_name, entries);
				},
			"e" | "edit" => edit( &args.next().unwrap(), entries),
			"r" | "rm" | "remove" => {
					let mut entry_name = "".to_owned();
					if let Some(arg1) = args.next() {
						entry_name.push_str(&arg1);
					} else {
						println!("[rm] no name supplied, exiting.");
						return;
					}
					remove(entry_name, &mut entries);
				},
			"ls" | "list" => list(&entries), // pxc ls category(optional)
			"pkg" => {
					if let Some(arg1) = args.next() {
						match &arg1[..] {
							"install" | "in" | "i" => println!("pkg not yet implemented"),
							"remove" | "rm" | "r" => println!("pkg not yet implemented"),
							_ => println!("cpkg: invalid arg {}", arg1)
						}
					} else {
						println!("[pkg] specify install or remove");
					}
				},
			_ => {
					// run cmd
					let opt_entry = get_entry_by_name(&arg, entries);
					
					match opt_entry {
						Some(ent) => {
							println!("running command '{}', filehash: {}", arg,  ent.filehash);
							let cmdpath = get_pxc_path().to_string() + "/cmd/" + &ent.filehash;
							println!("cmd path: '{}'", cmdpath);

							// set permission
							Command::new("chmod")
							.arg("777")
							.arg(&cmdpath)
							.status()
							.expect("failed to execute process");

							let mut cmdargs = "".to_owned();

							loop {
								match args.next() {
									Some(carg) => {
										cmdargs.push_str(&carg.to_owned());
										cmdargs.push_str(" ");
										
									},
									None => {break;}
								};
							}

							println!("carg: {}", cmdargs);

							// run
							Command::new("sh")
							.arg("-c")
							.arg(cmdpath + " " + &cmdargs)
							.spawn()
							.expect("failed to execute process");
						},
						None  => println!("command '{}' not found", arg)
					};
				}
		}
	} else {
		help();
	}
}

fn read_map_file(map: &str) -> Vec<MapEntry> {

	let mut result1: Vec<MapEntry> = Vec::new();
	let mut map_file = home::home_dir().unwrap().as_os_str().to_str().unwrap().to_owned();
	map_file.push_str("/.pxc/map/");
	map_file.push_str(map);

	if let Ok(result) = read_lines(map_file) {
		for line in result.flatten() {
			let parts = line.split(";");
			let test = parts.collect::<Vec<_>>();
			result1.push(MapEntry { 
				name: String::from(test[0]),
				category: String::from(test[1]),
				filehash: String::from(test[2])
				}
			);
		}
	} else {
		println!("pxc not initialized!");
		match init() {
			Ok(_) => {},
			Err(e) => {println!("Could not initialize!: {}", e)}
		}
	}

	return result1;
}

fn remove(entry_name: String, entries: &mut Vec<MapEntry>) {
	if !check_entry_exists(&entry_name, &entries) {
		println!("[rm] map entry with name '{}' doesn't exist!", entry_name);
		return;
	}

	entries.remove(entries.iter().position(|x| *&x.name == entry_name).expect("not found"));

	save_map(&entries);

	println!("[rm] removed {}!", entry_name);
}

fn check_entry_exists(entry_name: &str, entries: &Vec<MapEntry>) -> bool {
	for entry in entries {
		if entry.name == entry_name {
			return true;
		}
	}
	return false;
}

fn get_entry_by_name(entry_name: &str, entries: Vec<MapEntry>) -> Option<MapEntry> {
	for entry in entries {
		if entry.name == entry_name {
			return Some(entry);
		}
	}
	return None;
}


fn add(mut new_entry: MapEntry, entries: &mut Vec<MapEntry>) {
	if check_entry_exists(&new_entry.name, entries) {
		println!("[add] map entry with this name already exists!");
		return;
	}
	if new_entry.category == "" {
		new_entry.category = "default".to_string();
	}

	entries.push(new_entry);
	save_map(entries);
}

fn get_pxc_path() -> String {
	let mut map_path = "".to_owned();
	let mut homedir: String = "".to_owned();
	homedir.to_owned();

	match home::home_dir() {
		Some(path) if !path.as_os_str().is_empty() => homedir.push_str(path.as_os_str().to_str().unwrap()),
			_ => println!("Unable to get your home dir!"),
	}
	map_path.push_str(&homedir);
	map_path.push_str("/.pxc");

	return map_path;
}

fn save_map(entries: &Vec<MapEntry>) {

	if !cfg!(unix) {return; }

	let mut newfilepath = "".to_owned();
	let mut homedir: String = "".to_owned();
	homedir.to_owned();

	match home::home_dir() {
		Some(path) if !path.as_os_str().is_empty() => homedir.push_str(path.as_os_str().to_str().unwrap()),
			_ => println!("Unable to get your home dir!"),
	}

	// pxc directory: root pxec directory
	match fs::create_dir_all(homedir.clone() + "/.pxc") {
		Ok(()) => {},
		Err(dir) => {println!("error when creating dir {}", dir)}
	}
	// map directory: stores the mapping of command to script
	match fs::create_dir_all(homedir.clone() + "/.pxc/map/"){
		Ok(()) => {},
		Err(dir) => {println!("error when creating dir {}", dir)}
	}
	// commands directory: stores all script files
	match fs::create_dir_all(homedir.clone() + "/.pxc/cmd/"){
		Ok(()) => {},
		Err(dir) => {println!("error when creating dir {}", dir)}
	}

	newfilepath.push_str(&homedir);
	newfilepath.push_str("/.pxc/map/pxc");

	if !Path::new(&newfilepath).exists() {
		println!("[save] map file doesn't exist!");
		return;
	}

	println!("path {}", newfilepath);

	let path = newfilepath;
    let f = File::create(&path).expect("unable to create file");
    let mut f = BufWriter::new(f);

    for entry in entries {

		let entry_line = "".to_owned() + &entry.name + ";" + &entry.category + ";" + &entry.filehash;

		println!("ok, writing {} to file {}", entry_line, &path);

        write!(f, "{}\n", entry_line).expect("unable to write");
    }

	println!("[save] file saved!");

}

fn edit(mut entry_name: &str, entries: Vec<MapEntry>) {
	match get_entry_by_name(&entry_name, entries) {
		Some(ent) => {
			println!("[edit] editing command '{}', filehash: {}", entry_name,  ent.filehash);
			let cmdpath = get_pxc_path().to_string() + "/cmd/" + &ent.filehash;
			println!("cmd path: '{}'", cmdpath);

			Command::new("vim")
			.arg(cmdpath)
			.status()
			.expect("failed to execute process");
		},
		None  => println!("command '{}' not found", entry_name)
	};
}

fn list(entries: &Vec<MapEntry>) {
	for entry in entries.iter() {
		println!("[{}  ||  {}  ||  {}]",entry.name,entry.category,entry.filehash);
	}
}

fn read_lines<P>(filename: P) -> io::Result<io::Lines<io::BufReader<File>>>
where P: AsRef<Path>, {
	let file = File::open(filename)?;
	Ok(io::BufReader::new(file).lines())
}

fn init() -> std::io::Result<()> {

	println!("[init] initializing pxc..");

	let mut newfilepath = "".to_owned();
	let mut homedir: String = "".to_owned();
	homedir.to_owned();

	if cfg!(windows) {
		// windows todo
		return Ok(());

	} else if cfg!(unix) {

		match home::home_dir() {
			Some(path) if !path.as_os_str().is_empty() => homedir.push_str(path.as_os_str().to_str().unwrap()),
				_ => println!("Unable to get your home dir!"),
		}

		// pxc directory: root pxec directory
		match fs::create_dir_all(homedir.clone() + "/.pxc"){
			Ok(()) => {},
			Err(dir) => {println!("error when creating dir {}", dir)}
		}
		// map directory: stores the mapping of command to script
		match fs::create_dir_all(homedir.clone() + "/.pxc/map/"){
			Ok(()) => {},
			Err(dir) => {println!("error when creating dir {}", dir)}
		}
		// commands directory: stores all script files
		match fs::create_dir_all(homedir.clone() + "/.pxc/cmd/"){
			Ok(()) => {},
			Err(dir) => {println!("error when creating dir {}", dir)}
		}

	}

	newfilepath.push_str(&homedir);
	newfilepath.push_str("/.pxc/map/pxc");

	if Path::new(&newfilepath).exists() {
		println!("[init] file already exists");
		return Ok(());
	}

	println!("path {}", newfilepath);
	let mut file = File::create(&newfilepath)?;

	file.write_all(b"test;test;00000000!")?;

	println!("[init] init successful!");

	Ok(())
}


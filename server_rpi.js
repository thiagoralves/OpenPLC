var express = require("express");
var multer = require('multer');
var app = express();
var upload = multer({ dest: './ladder_files/'});
var spawn = require('child_process').spawn;
var openplc = spawn('./core/openplc');

var plcRunning = true;

app.use(multer({ dest: './ladder_files/',
    rename: function (fieldname, filename) 
    {
		return filename;
    },
	onFileUploadStart: function (file) 
	{
		console.log(file.originalname + ' is starting ...');
	},
	onFileUploadComplete: function (file) 
	{
		console.log(file.fieldname + ' uploaded to  ' + file.path);
		console.log('finishing old program...');
		openplc.kill('SIGTERM');
		plcRunning = false;
		compileLadder(file.originalname);
	}
}));

app.get('/',function(req,res)
{
	showMainPage(req,res);
});

app.get('/run',function(req,res)
{
	if (plcRunning == false)
	{
		console.log('Starting OpenPLC Software...');
		openplc = spawn('./core/openplc');
		plcRunning = true;
	}
	showMainPage(req,res);
});

app.get('/stop',function(req,res)
{
	if (plcRunning == true)
	{
		console.log('Stopping OpenPLC Software...');
		openplc.kill('SIGTERM');
		plcRunning = false;
	}
	showMainPage(req,res);
});

app.post('/api/upload',function(req,res)
{
    upload(req,res,function(err) 
    {
        if(err) 
        {
            return res.end("Error uploading file.");
        }
        showMainPage(req,res);
    });
});

app.listen(8080,function()
{
    console.log("Working on port 8080");
});

function showMainPage(req,res)
{
	var htmlString = '\
		<!DOCTYPE html>\
		<html>\
			<head>\
				<style>\
					body {background-color:lightgray}\
					h1   {color:Black}\
					p    {color:black}\
				</style>\
			</head>\
			<body>\
				<center><h1>OpenPLC on Raspberry Pi Server</h1></center>';
				if (plcRunning == true)
				{
					htmlString += '<p>Current PLC Status: <font color = "green">Running</font></p>';
				}
				else
				{
					htmlString += '<p>Current PLC Status: <font color = "red">Stopped</font></p>';
				}
				htmlString += '\
				<button type="button" onclick="location.href=\'run\';">Run</button>\
				<button type="button" onclick="location.href=\'stop\';">Stop</button>\
				<br><br><br>\
				<p>Change Ladder diagram:</p>\
				<form id        =  "uploadForm"\
					enctype   =  "multipart/form-data"\
					action    =  "/api/upload"\
					method    =  "post">\
					<input type="file" name="ladderDiagram" accept=".ld">\
					<input type="submit" value="Upload Ladder" name="submit">\
				</form>\
			</body>\
		</html>';
	
	res.send(htmlString);
}

function compileLadder(fileName)
{
	console.log('compiling ladder diagram...');
	var compiler = spawn('./OPLC_Compiler', ['./ladder_files/' + fileName]);
	
	compiler.stdout.on('data', function(data)
	{
		console.log('OPLC_Compiler: ' + data);
	});
	compiler.on('close', function(code)
	{
		if (code != 0)
		{
			console.log('Error compiling ladder diagram');
		}
		else
		{
			console.log('OPLC_Compiler exit code: ' + code);
			moveFiles();
		}
	});
}

function moveFiles()
{
	console.log('moving files...');
	var copier = spawn('mv', ['-f', 'ladder.cpp', './core/ladder.cpp']);
	copier.on('close', function(code)
	{
		if (code != 0)
		{
			console.log('error moving files');
		}
		else
		{
			compileOpenPLC();
		}
	});
}

function compileOpenPLC()
{
	console.log('compiling OpenPLC...');
	
	var exec = require('child_process').exec;
	exec('g++ -lrt -lwiringPi -lpthread ./core/*.cpp -o ./core/openplc', function(error, stdout, stderr) 
	{
		console.log('stdout: ' + stdout);
		console.log('stderr: ' + stderr);
		if (error !== null) 
		{
			console.log('exec error: ' + error);
			console.log('error compiling OpenPLC. Please check your ladder diagram');
		}
		else 
		{
			console.log('compiled without errors');
			console.log('Starting OpenPLC Software...');
			openplc = spawn('./core/openplc');
			plcRunning = true;
		}
	});
}

const csv = require('csv-parser');
const fs = require('fs');

//---------------------- Statistics Functions ----------------------//
//
//------------------------------------------------------------------//

function gm_log(a) {
    var b,c,d,e,f;
    for (cft = [76.18009172947146, -86.50532032941678, 24.01409824083091, -1.231739572450155, .001208650973866179, -5395239384953e-18], c = b = a, d = b + 5.5, d -= (b + .5) * Math.log(d), e = 1.000000000190015, f = 0; 5 >= f; f++)
        e += cft[f] / ++c;
    return -d + Math.log(2.5066282746310007 * e / b)
}
function gm_s(a, b, c) {
    var g,h,i,j,k,d = 100,e = 3e-7;
    for (j = a, i = h = 1 / a, g = 1; d >= g; g++)
        if (++j, i *= b / j, h += i, Math.abs(i) < Math.abs(h) * e)
            return k = h * Math.exp(-b + a * Math.log(b) - c);
    return console.log("Sorry, failed to find to a solution in gm_s"), -1
}
function gm_c(a, b, c) {
    var g,h,i,j,k,l,m,d = 100,e = 3e-7,f = 1e-30;
    for (h = b + 1 - a, i = 1 / f, j = 1 / h, l = j, m = 1; d >= m && (g = -m * (m - a), h += 2, j = g * j + h, f > Math.abs(j) && (j = f), i = h + g / i, f > Math.abs(i) && (i = f), j = 1 / j, k = j * i, l *= k, !(e > Math.abs(k - 1))); m++);
    return m > d ? (console.log("Sorry, failed to find to a solution in gm_c"), -1) : Math.exp(-b + a * Math.log(b) - c) * l
}
function gm_qb(a, b, c) {
    var d;
    if (a + 1 > b) {
        if (d = gm_s(a, b, c), 0 > d)
            return -1;
        d = 1 - d
    } else
        d = gm_c(a, b, c);
    return d
}
function gm_q(a, b) {
    var c,
        d = gm_log(a);
    return c = gm_qb(a, b, d)
}
function gm_qderiv(a, b, c) {
    var d;
    return d = -b + (a - 1) * Math.log(b) - c, -Math.exp(d)
}
function gm_qinv(a, b) {
    var e,f,g,h,i,c = 20,d = 2e-8,j = gm_log(a);
    for (f = a + Math.sqrt(a) * (.5 - b), e = 1; c >= e; e++) {
        if (g = gm_qb(a, f, j), 0 > g)
            return -1;
        if (g -= b, d > Math.abs(g))
            return f;
        h = gm_qderiv(a, f, j), i = g / h, f -= i
    }
    return console.log("Sorry, failed to find to a solution in gm_qinv"), -1
}
function chidist(a, b) {
    var c;
    return 0 > a ? (console.log("x cannot be negative"), -1) : 0 >= b || Math.floor(b) != b ? (console.log("Degrees of freedom must be a positive integer"), -1) : c = gm_q(b / 2, a / 2)
}
function chi2inv(a, b) {
    var c;
    return 0 >= a || a > 1 ? (console.log("p must be in the range (0,1]"), -1) : 1e-5 > Math.abs(a - 1) ? 0 : 0 >= b || Math.floor(b) != b ? (console.log("Degrees of freedom must be a positive integer"), -1) : (c = gm_qinv(Math.floor(b) / 2, a), 2 * c)
}



//---------------------- OBJECT MATH ----------------------//
var objMath = function( a, b, fn ){
	var c;
	if( typeof(b) === "object" ){ c=b; b=a; a=c; } //guarantee that a is the object (if there is one)
	if( typeof(a) !== "object" ){ return( fn(a,b) ); } //since neither a or b are objects, return their sum
	c = {};
	var field;
	if( typeof(b) === "object" ){
		for( field in a ){ c[field] = fn(a[field],b[field]); }
	} else {
		for( field in a ){ c[field] = fn(a[field],b); }
	}
	return( c );
}




//---------------------- Main Program Object----------------------//
//
// Data Structure:
// [{field1:"abc", field2:"def",...},{field1:"efg", field2:"hij",...},...]
//
//----------------------------------------------------------------//

function db(data){
	if( data ) this.data = data;
	else this.data = [];

	this.hashes = {};

	this.load = function( fileName, onload, headerLines ){
    let results = [];
    fs.createReadStream(fileName)
      .pipe(csv())
      .on('data', (data) => results.push(data) )
      .on('end', () => {
        this.data = results;
        if( onload ){ onload.call( this ) }
      })

    /*
    fs.readFile( fileName, 'binary', (err,data) => {
			if( err ) return( console.log(err) );
      if( fileName.substr(-3) == "csv" ){
        if( headerLines ){
          var lines = data.split('\n');
          var headings = lines[0].split(',');
          for( var i=1; i<headerLines; i++ ){
            var newHeadings = lines[i].split(',');
            for( var j=0; j<headings.length; j++ ){
              if( newHeadings[j] != "" ) headings[j] = newHeadings[j];
            }
          }
          lines[0] = headings.join(',');
          lines.splice(1,headerLines-1);
          var data = lines.join('\n');
        }
        self.data = $.csv.toObjects(data);
      } else {
        self.data = JSON.parse(data).data;
      }
      if( onload ){ onload.call( self ) }

		});
    */
	};
	this.keys = function(){ return( this.data.length > 0 ? Object.keys(this.data[0]) : null );}
	this.row = function( r ){ return( this.data[r] ); }
	this.add = function( rec ){ this.data.push(rec); this.hashes={}; };
	this.remove = function( id ){ this.data.splice( id, 1 ); this.hashes={}; };
  this.pop = function(){ return(this.data.pop()); };
	this.log = function(){console.log(this.data);};
	this.length = function(){return(this.data.length);}
  this.render = function($obj, onClick){
		if( this.data.length == 0 ){ $obj.html("The dataset is empty!"); return; }
		var $table = $("<table style='width:100%'/>").appendTo($obj);
		var $thead = $("<thead/>").appendTo($table);
		var $tbody = $("<tbody/>").appendTo($table);

		//Render the headers
		var $row = $("<tr/>").appendTo($thead);
		var $extraRow = $("<tr/>"); //Used when you need to create a splay table
		var hdrCount = 0;
		var includeExtraRow = false;
		for( field in this.data[0] ){
			if( typeof( this.data[0][field] ) == "object" ){
				//Determine the number of fields...
				var numFields = 0;
				for( f in this.data[0][field] ){
					numFields++;
					$("<th>"+f+"</th>").appendTo($row);
				}
				$("<th colspan="+numFields+">"+field+"</th>").appendTo($extraRow);
				includeExtraRow = true;
			} else {
				$("<th>"+field+"</th>").appendTo($row);
				$("<th></th>").appendTo($extraRow);
			}
		}
		if( includeExtraRow ) $extraRow.prependTo( $thead );

		//Render the table body
		for( var i=0; i<this.data.length; i++ ){
			var $row = $("<tr/>").appendTo($tbody);
			for( field in this.data[i] ){
				if( typeof( this.data[i][field] ) == "object" ){
					for( f in this.data[i][field] ){
						$("<td>"+this.data[i][field][f]+"</td>").appendTo($row);
					}
				} else {
					$("<td>"+this.data[i][field]+"</td>").appendTo($row);
				}
			}
			if( onClick ){
				$row.data("index", i);
				$row.click(function(){ onClick.call( self.data[$(this).data("index")], $(this).data("index") ) });
			}
		}

	},

	this.clear = function(){this.data = []; this.numRows = 0;};
	this.createIndex = function( fields, separator ){ //Create a hashed index of the data based on a set of fields
		var sep = separator || "|";
		var index = {};
		for( var i=0; i<this.data.length; i++ ){
			var hash = fields.length > 0 ? "" : "ALL";
			for( var j=0; j<fields.length; j++ ){ hash += this.data[i][fields[j]] + sep }
			hash = hash.substr(0, hash.length-sep.length);
			if( !index[hash] ) index[hash] = [];
			index[hash].push(this.data[i]);
		}
		return(index);
	};
	this.group = function( fields, combinations ){
		var result = new db();
		var index = this.createIndex( fields );
		var i, field;

		//Helper functions used in the for loop later
		var calcSum = function(a,b){return(Number(a)+Number(b))}
		var calcMax = function(a,b){return(Math.max(a,b))}
		var calcMin = function(a,b){return(Math.min(a,b))}
		var calcAvg = (a,b)=>{return(parseFloat(a)+parseFloat(b)/recList.length)}
		var calcStDev1 = (a,b)=>{return((a-b)*(a-b) )}
		var calcStDev2 = (a,b)=>{return( Math.sqrt( a / (b-1) ) )}

		for( var hash in index ){
			var recList = index[hash];
			var rec = {};
			for( i=0; i<fields.length; i++ ){ rec[fields[i]] = recList[0][fields[i]] } //Add known fields

			var averages = {};
			var counts = {};

			for( field in combinations ){
				averages[field] = 0;
				counts[field] = 0;

				if( typeof(recList[0][field]) === "object" ){
					rec[field] = {};
					for( var fName in recList[0][field] ){
						rec[field][fName]= (combinations[field]==="min") ? 9999999999999 : 0;
					}
				} else {
					rec[field] = (combinations[field]==="min") ? 9999999999999 : 0;
				}
			}


			for( i=0; i<recList.length; i++ ){
				for( field in combinations ){
					switch( combinations[field] ){
						case "count": rec[field]++; break;
						case "sum":   rec[field]=objMath(rec[field],recList[i][field],calcSum); break;
						case "max":   rec[field]=objMath(rec[field],recList[i][field],calcMax); break;
						case "min":   rec[field]=objMath(rec[field],recList[i][field],calcMin); break;
						case "avg":   rec[field]=objMath(rec[field],recList[i][field],calcAvg); break;
						case "stdev":
							averages[field] = objMath(averages[field],recList[i][field],calcAvg);
							counts[field]++;
							break;
						case "override": rec[field]=recList[i][field]; break;
						default: break;
					}
				}
			}

			for( field in combinations ){
				if( combinations[field] == "stdev" ){
					for( i=0; i<recList.length; i++ ){
						rec[field]+=objMath(recList[i][field],averages[field],calcStDev1);
					}
					rec[field] = objMath(rec[field],counts[field],calcStDev2);
				}
			}


			result.add( rec );
		}
		return( result );
	};
	this.merge = function( db, keyField ){
		var result = this.clone();
		var keys1 = this.keys();
		var keys2 = db.keys();

		var newKeys = keys2.filter(value => -1 == keys1.indexOf(value));
		newKeys.map( key => {
			result.addColumn( key, 0 )
		});

		var index = db.createIndex( [keyField] );

		for( var i = 0; i<result.data.length; i++ ){
			if( index[result.data[i][keyField]] ){
				var rec = index[result.data[i][keyField]][0];
				for( var k = 0; k<newKeys.length; k++ ){
					result.data[i][newKeys[k]] = rec[newKeys[k]];
				}
			}
		}
		return( result );
	};
	this.pivot = function( keyFields, pivotField, groupingFields ){
		var result = new db();
		var index = this.createIndex( keyFields );

		var i, field;

		var pivotList = {}; //Construct a list of pivot headers
		for( i=0; i<this.data.length; i++ ){ pivotList[this.data[i][pivotField]]=0; }

		console.log( pivotList );

		for( var hash in index ){
			var recList = index[hash];

			var rec = {};

			//Add known fields
			for( i=0; i<keyFields.length; i++ ){ rec[keyFields[i]] = recList[0][keyFields[i]] } //Add known fields

			//Add blank pivots
			for( field in groupingFields ){ rec[field] = $.extend( true, {}, pivotList ); }

			console.log( rec );

			//Calculate the average for the standard deviation calculation
			var averages = {}
			for( field in groupingFields ){
				averages[field] = 0;
				recList.map(v=>averages[field]+=parseFloat(v)/recList.length);
			}


			for( i=0; i<recList.length; i++ ){

				var pivotVal = recList[i][pivotField]

				for( field in groupingFields ){
					switch( groupingFields[field] ){
						case "count": rec[field][pivotVal]++; break;
						case "sum":   rec[field][pivotVal]+=parseFloat(recList[i][field]); break;
						case "max":   rec[field][pivotVal]=Math.max(rec[field][pivotVal],recList[i][field]); break;
						case "min":   rec[field][pivotVal]=Math.min(rec[field][pivotVal],recList[i][field]); break;
						case "avg":   rec[field][pivotVal]+=parseFloat(recList[i][field])/recList.length; break;
						case "stdev": rec[field][pivotVal]+=(averages[field]-parseFloat(recList[i][field])) * (averages[field]-parseFloat(recList[i][field])) ; break;
						default: break;
					}
				}
			}

			console.log( rec );

			for( field in groupingFields ){
				for( pivotVal in rec[field] ){

				}
			}

			result.add( rec );
		}

		return(result);
	};

	this.invert = function( pivotField, headerColName ){
		var result = new db();
		var index = this.createIndex( [pivotField] );
		var keys = this.keys();

		keys.map( key => {
			var row = {};
			row[headerColName] = key;
			for( hash in index ){
				row[hash]=index[hash][0][key];
			}
			if( key != pivotField ) result.data.push( row );
		});
		return( result );
	};

	this.filter = function( rules, flip ){
		flip = flip || 0;
		var result = new db();
		var isTrue = false;
		for( var i=0; i<this.data.length; i++ ){
			var include = true;
			for( var field in rules ){
				if( typeof( rules[field] ) === "function" ){
					isTrue = rules[field]( this.data[i][field] );
				} else {
					var arrRules = (Array.isArray(rules[field])) ? rules[field] : (rules[field]==undefined) ? [undefined] : rules[field].split("|");
					isTrue = false;
					for( var j=0; j<arrRules.length; j++ ){
						isTrue = (this.data[i][field] === arrRules[j]) ^ flip;
						if( isTrue ) break;
					}
				}
				include &= isTrue;
				if( !include ) break;
			}
			if( include ){
				result.add(this.data[i]);
			}
		}
		return( result );
	};
	this.do = function( fn ){
		for( var i=0; i<this.data.length; i++ ){
			fn.call(this, this.data[i], i);
		}
		return( this ); //allow chaining
	};

	this.sort = function( key, ascending ){
		var v = ascending ? 1 : -1;
		if( isNaN( parseFloat(this.data[0][key]) ) ){
			this.data.sort( (a,b) => a[key] >= b[key] ? v : -v );
		} else {
			this.data.sort( (a,b) => parseFloat(a[key]) >= parseFloat(b[key]) ? v : -v );
		}
		return( this );
	};

	this.collapse = function( rule ){
		rule = rule || "sum"; // by default, sum up all of the rows
		var result = new db();
		var rec = {}; //set up a blank record to hold all of the columns
		for( let field in this.data[0] ){ rec[field] = null; }

		if( rule == "stdev") var averages = this.collapse( "avg" ).data[0]; //get the average for a standard deviation calculation
		this.data.map( (row) => {
			for( let field in row ){
				if( isNaN(row[field]) ) continue; //We only want numbers!
				switch( rule ){
					case "count": rec[field]++; break;
					case "sum":   rec[field]+=row[field] ? parseFloat(row[field]) : 0; break;
					case "max":   rec[field]=Math.max(rec[field],parseFloat(row[field])); break;
					case "min":   rec[field]=Math.min(rec[field],parseFloat(row[field])); break;
					case "avg":   rec[field]+=parseFloat(row[field])/this.data.length; break;
					case "stdev":   rec[field]+=(averages[field]-parseFloat(row[field]))*(averages[field]-parseFloat(row[field])); break;
					default: break;
				}
			}
		});
		if( rule == "stdev" ){
			for( let field in rec ){
				if( isNaN(rec[field]) ) continue; //We only want numbers!
				rec[field] = Math.sqrt(rec[field]/(this.data.length-1));
			}
			result.add(rec);
			result.add(averages);
		} else {
			result.add(rec);
		}
		return( result );
	};

	this.blankRow = function(){ //returns an object based on first row of data. All strings are "" all numbers are 0
		var data = extend( true, {}, this.data[0] );
		for( let cell in data ){ data[cell] = null; }
		return(data)
	};

	this.addColumn = function( field, defaultValue ){
		if( defaultValue == undefined ) defaultValue = null;
		this.data.map( r => { r[field] = defaultValue } ); /* initialize fields to zero */
		return( this );
	};
	this.renameColumn = function( oldKey, newKey ){
		this.do( (row,i) => {
			row[newKey] = row[oldKey];
			delete( row[oldKey] );
		});
	};
	this.copyColumn = function( field, newField ){
		this.data.map( r => { r[newField] = r[field] } ); /* initialize fields to zero */
		return( this );
	};
	this.removeColumn = function( key ){
		this.do( (row,i) => delete( row[key] ) );
		return( this );
	};
	this.expandColumn = function( key ){
		if( typeof( this.data[0][key] ) != "object" ) return; // make sure the column has sub objects and needs to be expanded
		var keys = Object.keys(this.data[0][key]); //fetch the names of the sub keys
		this.data.map( r => {
			keys.map( k => r[k] = r[key][k] );
			delete(r[key]);
		});
		return( this );
	};

	this.union = function( _db ){ //unify the current dataset with another
		let _row = _db.blankRow(); //Create a blank version of the new db for reference

		if( this.data.length > 0 ){
			for( let field in _row ){ //add any new columns from db into the current recordset
				if( this.data[0][field] === undefined ){
					this.addColumn(field);
				}
			}

			for( let field in this.data[0] ){ //add any new columns from current recordset into db
				if( _row[field] === undefined ){
					_db.addColumn(field);
				}
			}
		}

		var newDB = new db(); //unify the two datasets
		this.data.map( row => {newDB.add(row)} );
		_db.data.map( row => {newDB.add(row)} );

		return( newDB );
	};
	this.clone = function(){
		return( new db(JSON.parse(JSON.stringify(this.data)) ) );
	}

	this.update = function(updates, filter){
		let numUpdates = 0;
		this.data.map( (row,i) => {
			let isTrue = true;
			for( let field in filter ){
				isTrue &= (row[field] === filter[field]);
				if( !isTrue ) return; // if not true, move onto the next record
			}
			numUpdates++;
			for( let field in updates ){
				row[field] = updates[field]
			}
		});
		return( numUpdates );
	};

	this.delete = function(filters){
		var newDB = this.clone();
		newDB.data.map( (row,i) => {
			let isTrue = true;
			for( let field in filters ){
				isTrue &= (row[field] === filters[field]);
				if( !isTrue ) return; // if not true, move onto the next record
			}
			newDB.remove(i);
		});
		return( newDB );
	};

	this.hash = function(field){ //Creates a hash table with pointers to the rows. Field should be unique!
		if( this.hashes[field] ) return;
		this.hashes[field] = {};
		this.data.map( (row,i) => { this.hashes[field][row[field]] = row; })
	};

	this.findRow = function(field, value){ //Returns a row reference where field == value;
		this.hash(field);
		if( !this.hashes[field][value] ) return( false );
		return( this.hashes[field][value] );
	};

	this.override = function( _db, keyField ){ //Returns new table with existing data overridden with new data
		var newDB = this.clone();

		_db.data.map( (row,i) => {
			let _row = newDB.findRow( keyField, row[keyField])
			if( !_row ){
				_row = newDB.blankRow();
				newDB.add(_row); // push the reference onto the db
			}
			for( let cell in row ){
				if( row[cell] !== null ) _row[cell] = row[cell];
			}
		})
		return( newDB );
	};

	this.answerSet = function(){
		let keys = this.keys();
		let ansSet = {};
		keys.map( (key)=>{
			ansSet[key]={};
			this.data.map( (row,i)=>{
				let ans = row[key];
				ansSet[key][ans] = (ansSet[key][ans]===undefined) ? 1 : ansSet[key][ans] + 1;
			});
		});
		return( ansSet );
	};

	this.pcc = function(field1, field2){
		// Calculates Pearsons Correlation (R) between two numeric variables
		let avgs = this.data.collapse("avg"); //Calculate average values for the rows #inefficient... but wa-eva
		let f1_avg = avgs[field1];
		let f2_avg = avgs[field2];

		let p1 = this.data.map( (row) => (row[field1] - f1_avg) * (row[field2] - f2_avg) );
		let p2 = this.data.map( (row) => (row[field1] - f1_avg) * (row[field1] - f1_avg) );
		let p3 = this.data.map( (row) => (row[field2] - f2_avg) * (row[field2] - f2_avg) );
		console.log( "f1 avg:" + f1_avg );
		console.log( "f2 avg:" + f2_avg );
		console.log( "p1:" + p1 );
		console.log( "p2:" + p2 );
		console.log( "p3:" + p3 );
		let r = p1 / (Math.sqrt(p2) + Math.sqrt(p3))
		console.log( r );
	};

	//chisqr computes the covariance probability of two columns
	//field1: independent variable
	//field2: dependent variable
	//f1: optional object containing possible answers in order {"ans1":0, "ans2":0,...}
	//f2: optional object containing possible answers in order {"ans1":0, "ans2":0,...}
	this.chisqr = function(field1, field2, f1, f2){

		f1 = f1===undefined ? {} : f1; // store counts of field1
		f2 = f2===undefined ? {} : f2; // store counts of field2
		let f12 = {}; // store counts of field1/field2 combinations
		let totalRows = this.data.length; // total of all rows

		//Generate counts
		this.data.map( row => {
			if( f1[row[field1]]===undefined ){ f1[row[field1]] = 1; } else { f1[row[field1]]++; } //count instances of field 1
			if( f2[row[field2]]===undefined ){ f2[row[field2]] = 1; } else { f2[row[field2]]++; } //count instances of field 2
			let field12 = row[field1]+"|"+row[field2];
			if( f12[field12]===undefined ){ f12[field12] = 1; } else { f12[field12]++; } //count instances of each combo
		});

		let chi2 = 0;
		for( ind in f1 ){ //loop through independent variable
			for( dep in f2 ){ //loop through dependent variable
				let obsValue = f12[ind+"|"+dep] || 0; // If observed value exists, use it. otherwise use zero
				let expValue = f1[ind] / totalRows * f2[dep]; //Derive an expected average value
				chi2 += (obsValue - expValue)*(obsValue - expValue)/expValue;
			}
		}
		df = (Object.keys(f1).length-1) * (Object.keys(f2).length-1); // Calculate the degrees of freedom

		return( {chisqr:chi2, degrees:df, p:chidist(chi2,df).toFixed(5)} )
	};

	this.chartData = function(field1, field2, f1, f2){
		f1 = f1===undefined ? {} : f1; // store counts of field1
		f2 = f2===undefined ? {} : f2; // store counts of field2
		let f12 = {}; // store counts of field1/field2 combinations
		let totalRows = this.data.length; // total of all rows

		//Generate counts
		this.data.map( row => {
			if( f1[row[field1]]===undefined ){ f1[row[field1]] = 1; } else { f1[row[field1]]++; } //count instances of field 1
			if( f2[row[field2]]===undefined ){ f2[row[field2]] = 1; } else { f2[row[field2]]++; } //count instances of field 2
			let field12 = row[field1]+"|"+row[field2];
			if( f12[field12]===undefined ){ f12[field12] = 1; } else { f12[field12]++; } //count instances of each combo
		});

		var dsCount = [];
		var dsPercent = [];

		for( let ind in f1 ){ //loop through independent variable
			let _dsCount = {name:ind, data:[]};
			let _dsPercent = {name:ind, data:[]};
			for( let dep in f2 ){ //loop through dependent variable
				let obsValue = f12[ind+"|"+dep] || 0; // If observed value exists, use it. otherwise use zero
				_dsCount.data.push( obsValue );
				_dsPercent.data.push( obsValue / f1[ind] );
			}
			dsCount.push( _dsCount );
			dsPercent.push( _dsPercent );
		}
		let xAxis = Object.keys(f1).map( (v)=>{ return(v === "" ? "--" : v) } );
		let yAxis = Object.keys(f2).map( (v)=>{ return(v === "" ? "--" : v) } );

		return( { counts:dsCount, percents:dsPercent, xAxis:xAxis, yAxis:yAxis } )
	};

};


module.exports = db;

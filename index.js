const fs = require('fs');
const path = require('path');

const resistorValues = fs.readFileSync(path.resolve('nte-resistor-values.csv'))
	.toString()
	.split('\n')
	.map(n => parseFloat(n));

const maxError = (Vin, Rref, stepCount) => {
	const R = value => {
		value.R = Rref * ((Vin / value.Vout) - 1);
		return value;
	};
	const Vout = (value, i) => {
		value.Vout = parseFloat((((i + 1) * (3.3 / stepCount)) - (3.3 / stepCount / 2)).toFixed(3));
		return value;
	};
	const nearestNTE = value => {
		if (value.Rdiff) {
			value.nearestNTE = resistorValues.reduce(function(prev, curr) {
				return (Math.abs(curr - value.Rdiff) < Math.abs(prev - value.Rdiff) ? curr : prev);
			})
		} else {
			value.nearestNTE = 0;
		}
		return value;
	};

	const Values = Array
		.apply(null, Array(stepCount))
		.map(a => ({
			Vout: null,
			R: null,
			Rdiff: null,
			nearestNTE: null,
			NTE_R: null,
			NTE_Vout: null,
		}));

	// console.log(Values);

	Values
		.map(Vout);

	// console.log(Values);

	Values
		.map(R);

	// console.log(Values);

	Values
		.map((value, i) => {
			if (Values[i + 1]) {
				value.Rdiff = value.R - Values[i + 1].R;
			}
			return value;
		});

	// console.log(Values);

	Values
		.map(nearestNTE);

	// console.log(Values);

	Values
		.map((value, i) => {
			value.NTE_R = Values
				.slice(i, Values.length)
				.reduce((a, b) => {
					return (a.nearestNTE || a) + b.nearestNTE;
				}, 0);
			return value;
		});

	// console.log(Values);

	Values
		.map((value) => {
			value.NTE_Vout = ((Rref / (Rref + value.NTE_R)) * Vin);
			return value;
		});

	// console.log(Values);

	Values
		.map((value) => {
			value.Vdiff = Math.abs((value.Vout - value.NTE_Vout) / Vin * 100);
			return value;
		});

	console.log(Values);

	Values.pop();

	// console.log(Values);

	const maxError = Values.reduce((a, b) => Math.max((a.Vdiff || a), b.Vdiff), 0);

	// console.log(maxError);

	return maxError;
};

// const maxErrors = [];

// for (var i = 10; i < 100000; i++) {
// 	maxErrors.push({
// 		Rref: i,
// 		maxError: maxError(3.3, i, 21)
// 	});
// };

// const minError = maxErrors.reduce((a, b) => Math.min((a.maxError || a), b.maxError), 100);

// maxErrors.forEach((i) => {
// 	if (i.maxError === minError) {
// 		console.log(i);
// 	}
// });

console.log(maxError(3.3, 211, 21))












/*
Timetable - Add print report buttons
v02 - Print repost 1 class per page, 2 teachers per page
v01 -251003 - initial version - seems to work
*/

// Add CSS styles for printing and layout
const styleCSS = `
<style>
  @media print {
    .report-group {
      page-break-inside: avoid; /* Keep entire group on one page */
      margin-bottom: 20px;
    }
    .reports-wrapper {
      display: flex;
      flex-wrap: wrap;
      justify-content: space-between;
    }
    .report-container {
      width: 48%; /* Two per row */
      box-sizing: border-box;
      margin-bottom: 20px;
    }
  }
  body { font-family: Arial; margin: 20px;  }
  table.grid { border-collapse: collapse; width: 100%; margin-bottom: 10px; }
  table.grid th, table.grid td { border: 1px solid #999; padding: 5px; text-align: center; min-width: 80px; height: 30px; }
  table.grid .slot { background: #f9f9f9; }
  table.grid .occupied { color: #000; }
</style>
`;

// Function to create the report buttons
function createReportButtons() {
    const controlsDiv = document.getElementById('controls'); // Target the controls div
    
    const classesReportButton = document.createElement('button');
    classesReportButton.id = 'classesReportBtn';
    classesReportButton.textContent = 'Generate Classes Report';
    classesReportButton.onclick = generateClassesReport;
    
    const teachersReportButton = document.createElement('button');
    teachersReportButton.id = 'teachersReportBtn';
    teachersReportButton.textContent = 'Generate Teachers Report';
    teachersReportButton.onclick = generateTeachersReport;
    
    controlsDiv.appendChild(classesReportButton);
    controlsDiv.appendChild(teachersReportButton);
}

// Function to generate and print the classes report
function generateClassesReport() {
    let reportContent = '<h1>All Classes Schedule</h1>';

    reportContent += '<div class="reports-wrapper">';

/*
    // Wrap every two classes in a group
    for (let i = 0; i < classes.length; i += 2) {
        reportContent += '<div class="report-group">';
        for (let j = i; j < i + 2 && j < classes.length; j++) {
            const c = classes[j];
            reportContent += `<div class="report-container">`;
            reportContent += `<h2>Class ${c.class_id || 'Unknown'}</h2>`;
            reportContent += '<div>'; // Wrapper for table
            reportContent += buildReportTableForClass(c.class_id);
            reportContent += '</div></div>'; // close report-container
        }
        reportContent += '</div>'; // close report-group
    }
*/
	//Wrap one class per page
	for (let i = 0; i < classes.length; i++) {
	    reportContent += '<div class="report-group">';
	    const c = classes[i];
	    reportContent += `<div class="report-container">`;
	    reportContent += `<h2>Class ${c.class_id || 'Unknown'}</h2>`;
	    reportContent += '<div>'; // Wrapper for table
	    reportContent += buildReportTableForClass(c.class_id);
	    reportContent += '</div></div>'; // close report-container
	    reportContent += '</div>'; // close report-group
	}


    reportContent += '</div>'; // close reports-wrapper

    // Open print window
    const printWindow = window.open('', '_blank');
    printWindow.document.write(`
      <html>
      <head>${styleCSS}</head>
      <body>${reportContent}</body>
      </html>
    `);
    printWindow.document.close();
    printWindow.print();
}

// Helper function to build table for a class
function buildReportTableForClass(cid) {
    let tableHTML = '<table class="grid">';
    let head = '<tr><th>Period</th>';
    days.forEach(d => head += `<th>${d}</th>`);
    head += '</tr>';
    tableHTML += head;

    slots.forEach(s => {
        let tr = '<tr>';
        tr += `<th>P${s}</th>`;
        days.forEach(d => {
            let tid = (timetable[cid] && timetable[cid][d] && timetable[cid][d][s]) || '';
            let cellClass = 'slot';
            let content = '';
            if (tid && teacherMap[tid]) {
                cellClass += ' occupied';
                content = teacherMap[tid].name;
                if (teacherMap[tid].color) {
                    content = `<span style="background-color: ${teacherMap[tid].color}; padding: 5px; display: block;">${content}</span>`;
                }
            }
            tr += `<td class="${cellClass}">${content}</td>`;
        });
        tr += '</tr>';
        tableHTML += tr;
    });
    tableHTML += '</table>';
    return tableHTML;
}

// Function to generate and print the teachers report
function generateTeachersReport() {
    let reportContent = '<h1>Teachers Weekly Program</h1>';

    reportContent += '<div class="reports-wrapper">';

    // Wrap every two teachers in a group
    for (let i = 0; i < teachers.length; i += 2) {
        reportContent += '<div class="report-group">';
        for (let j = i; j < i + 2 && j < teachers.length; j++) {
            const t = teachers[j];
            const tid = t.teacher_id;
            reportContent += `<div class="report-container">`;
            reportContent += `<h2>Teacher: ${t.name}</h2>`;
            reportContent += '<table class="grid">';
            reportContent += '<tr><th>Period</th>';
            days.forEach(d => reportContent += `<th>${d}</th>`);
            reportContent += '</tr>';

            // For each slot, create a row
            slots.forEach(s => {
                let tr = '<tr><th>P' + s + '</th>';
                days.forEach(d => {
                    let cellContent = 'Unassigned';
                    let cellClass = 'slot';

                    // Check all classes for assignment
                    classes.forEach(c => {
                        if (timetable[c.class_id] && timetable[c.class_id][d] && timetable[c.class_id][d][s] === tid) {
                            cellContent = c.class_id;
                            cellClass += ' occupied';
                            if (t.color) {
                                cellContent = `<span style="background-color: ${t.color}; padding: 5px; display: block;">${cellContent}</span>`;
                            }
                        }
                    });
                    tr += `<td class="${cellClass}">${cellContent}</td>`;
                });
                tr += '</tr>';
                reportContent += tr;
            });
            reportContent += '</table></div>'; // close report-container
        }
        reportContent += '</div>'; // close report-group
    }

    // Open print window
    const printWindow = window.open('', '_blank');
    printWindow.document.write(`
      <html>
      <head>${styleCSS}</head>
      <body>${reportContent}</body>
      </html>
    `);
    printWindow.document.close();
    printWindow.print();
}

// Initialize on DOM load
document.addEventListener('DOMContentLoaded', function() {
    createReportButtons();
    // existing other code...
});
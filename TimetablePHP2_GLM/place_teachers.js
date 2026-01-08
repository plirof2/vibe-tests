/*
 * place_teachers.js - Auto-place teachers in timetable
 * 
 * Features:
 * - Assign first N teachers as class-specific (exclusive to their class)
 * - Distribute remaining teachers round-robin across all free slots
 * - Respect max_hours, unavailable_days, and no consecutive hours rule
 * - Confirmation dialog before clearing all slots
 */

(function() {
  'use strict';

  let teacherUsedHours = {};

  function createAutoPlaceButton() {
    const controlsDiv = document.getElementById('controls');
    if (!controlsDiv) {
      console.error('Controls div not found');
      return;
    }

    const autoPlaceBtn = document.createElement('button');
    autoPlaceBtn.id = 'autoPlaceBtn';
    autoPlaceBtn.textContent = 'Auto Place Teachers';
    autoPlaceBtn.style.marginLeft = '10px';
    autoPlaceBtn.style.backgroundColor = '#28a745';
    autoPlaceBtn.style.color = 'white';
    autoPlaceBtn.style.cursor = 'pointer';

    autoPlaceBtn.onclick = showConfirmationDialog;
    controlsDiv.appendChild(autoPlaceBtn);
  }

  function showConfirmationDialog() {
    const modal = document.createElement('div');
    modal.id = 'autoPlaceConfirmModal';
    modal.style.cssText = `
      position: fixed;
      top: 0;
      left: 0;
      width: 100%;
      height: 100%;
      background: rgba(0,0,0,0.5);
      display: flex;
      justify-content: center;
      align-items: center;
      z-index: 10000;
    `;

    const modalContent = document.createElement('div');
    modalContent.style.cssText = `
      background: white;
      padding: 30px;
      border-radius: 8px;
      text-align: center;
      max-width: 500px;
      box-shadow: 0 4px 20px rgba(0,0,0,0.3);
    `;

    const title = document.createElement('h3');
    title.textContent = 'Auto Place Teachers';
    title.style.marginTop = '0';
    title.style.color = '#dc3545';

    const message = document.createElement('p');
    message.textContent = 'This will CLEAR ALL current teacher assignments and automatically place teachers based on these rules:';
    message.style.marginBottom = '15px';

    const rules = document.createElement('ul');
    rules.style.textAlign = 'left';
    rules.style.margin = '15px 0';
    rules.style.paddingLeft = '20px';
    rules.innerHTML = `
      <li>First N teachers will be class-specific (exclusive to one class)</li>
      <li>Remaining teachers will be distributed evenly across all classes</li>
      <li>Respect each teacher\'s max_hours limit</li>
      <li>Respect unavailable_days for each teacher</li>
      <li>No 2 consecutive hours for any teacher (rest period)</li>
      <li>Spread hours evenly across all days</li>
    `;

    const warning = document.createElement('p');
    warning.textContent = '⚠️ This action cannot be undone!';
    warning.style.color = '#dc3545';
    warning.style.fontWeight = 'bold';
    warning.style.margin = '15px 0';

    const buttonContainer = document.createElement('div');
    buttonContainer.style.marginTop = '20px';

    const cancelButton = document.createElement('button');
    cancelButton.textContent = 'Cancel';
    cancelButton.style.cssText = `
      background: #6c757d;
      color: white;
      padding: 10px 20px;
      border: none;
      border-radius: 4px;
      cursor: pointer;
      margin-right: 10px;
      font-size: 14px;
    `;

    const proceedButton = document.createElement('button');
    proceedButton.textContent = 'Proceed';
    proceedButton.style.cssText = `
      background: #dc3545;
      color: white;
      padding: 10px 20px;
      border: none;
      border-radius: 4px;
      cursor: pointer;
      font-size: 14px;
    `;

    cancelButton.onclick = () => {
      document.body.removeChild(modal);
    };

    proceedButton.onclick = () => {
      document.body.removeChild(modal);
      autoPlaceTeachers();
    };

    buttonContainer.appendChild(cancelButton);
    buttonContainer.appendChild(proceedButton);
    modalContent.appendChild(title);
    modalContent.appendChild(message);
    modalContent.appendChild(rules);
    modalContent.appendChild(warning);
    modalContent.appendChild(buttonContainer);
    modal.appendChild(modalContent);
    document.body.appendChild(modal);
  }

  function clearAllSlots() {
    showMsg('Clearing all slots...', true, 'status2');

    document.querySelectorAll('.slot').forEach(slot => {
      slot.textContent = '';
      slot.style.background = '#f9f9f9';
      slot.className = 'slot';
      delete slot.dataset.tid;
      slot.draggable = false;
      slot.title = '';
    });

    timetable = {};
    classes.forEach(c => {
      timetable[c.class_id] = {};
      days.forEach(d => {
        timetable[c.class_id][d] = {};
        slots.forEach(s => {
          timetable[c.class_id][d][s] = '';
        });
      });
    });
  }

  function findSlot(classId, day, slotNum) {
    const selector = `.slot[data-class-id="${classId}"][data-day="${day}"][data-slot="${slotNum}"]`;
    return document.querySelector(selector);
  }

  function isSlotFree(classId, day, slotNum) {
    const slot = findSlot(classId, day, slotNum);
    return slot && !slot.dataset.tid;
  }

  function wouldCreateConsecutive(teacherId, classId, day, slotNum) {
    const prevSlotNum = slotNum - 1;
    const nextSlotNum = slotNum + 1;

    if (prevSlotNum >= 1) {
      const prevSlot = findSlot(classId, day, prevSlotNum);
      if (prevSlot && prevSlot.dataset.tid === teacherId) {
        return true;
      }
    }

    if (nextSlotNum <= 9) {
      const nextSlot = findSlot(classId, day, nextSlotNum);
      if (nextSlot && nextSlot.dataset.tid === teacherId) {
        return true;
      }
    }

    return false;
  }

  function isTeacherUnavailable(teacherId, day) {
    const teacher = teacherMap[teacherId];
    if (!teacher || !teacher.unavailable_days) return false;

    const unavailableDays = teacher.unavailable_days.split('|').filter(d => d.trim() !== '');
    return unavailableDays.includes(day);
  }

  function isTeacherAssignedElsewhere(teacherId, day, slotNum, excludeClassId) {
    for (const c of classes) {
      if (c.class_id === excludeClassId) continue;

      const slot = findSlot(c.class_id, day, slotNum);
      if (slot && slot.dataset.tid === teacherId) {
        return true;
      }
    }
    return false;
  }

  function canPlaceTeacher(teacherId, classId, day, slotNum) {
    const teacher = teacherMap[teacherId];
    if (!teacher) return false;

    const currentHours = teacherUsedHours[teacherId] || 0;
    if (currentHours >= teacher.max_hours) {
      return false;
    }

    if (!isSlotFree(classId, day, slotNum)) {
      return false;
    }

    if (wouldCreateConsecutive(teacherId, classId, day, slotNum)) {
      return false;
    }

    if (isTeacherUnavailable(teacherId, day)) {
      return false;
    }

    if (isTeacherAssignedElsewhere(teacherId, day, slotNum, classId)) {
      return false;
    }

    return true;
  }

  function assignTeacherToSlot(teacherId, classId, day, slotNum) {
    const teacher = teacherMap[teacherId];
    if (!teacher) return false;

    const slot = findSlot(classId, day, slotNum);
    if (!slot) return false;

    slot.textContent = teacher.name;
    slot.style.background = teacher.color;
    slot.className = 'slot occupied';
    slot.dataset.tid = teacherId;
    slot.draggable = true;
    slot.title = '';

    if (!timetable[classId]) timetable[classId] = {};
    if (!timetable[classId][day]) timetable[classId][day] = {};
    timetable[classId][day][slotNum] = teacherId;

    teacherUsedHours[teacherId] = (teacherUsedHours[teacherId] || 0) + 1;

    return true;
  }

  function createUnassignedSlotsList() {
    const slotList = [];
    days.forEach(day => {
      slots.forEach(slotNum => {
        classes.forEach(c => {
          slotList.push({
            class_id: c.class_id,
            day: day,
            slot: slotNum
          });
        });
      });
    });
    return slotList;
  }

  function placeClassSpecificTeachers() {
    showMsg('Placing class-specific teachers...', true, 'status2');

    const numClasses = classes.length;
    const numClassSpecificTeachers = Math.min(teachers.length, numClasses);

    for (let i = 0; i < numClassSpecificTeachers; i++) {
      const teacher = teachers[i];
      const targetClass = classes[i];
      const teacherId = teacher.teacher_id;

      teacherUsedHours[teacherId] = 0;

      const unassignedSlots = createUnassignedSlotsList();
      let placed = 0;

      for (const slotInfo of unassignedSlots) {
        if (slotInfo.class_id !== targetClass.class_id) continue;

        if (teacherUsedHours[teacherId] >= teacher.max_hours) break;

        if (canPlaceTeacher(teacherId, slotInfo.class_id, slotInfo.day, slotInfo.slot)) {
          if (assignTeacherToSlot(teacherId, slotInfo.class_id, slotInfo.day, slotInfo.slot)) {
            placed++;
          }
        }
      }

      if (placed < teacher.max_hours) {
        console.warn(`Teacher ${teacher.name} could only be placed for ${placed}/${teacher.max_hours} hours`);
      }
    }
  }

  function placeDistributedTeachers() {
    showMsg('Placing distributed teachers...', true, 'status2');

    const numClasses = classes.length;
    const startIndex = Math.min(teachers.length, numClasses);

    if (startIndex >= teachers.length) {
      showMsg('All teachers assigned as class-specific', true, 'status2');
      return;
    }

    const distributedTeachers = teachers.slice(startIndex);
    const unassignedSlots = createUnassignedSlotsList();

    for (const teacher of distributedTeachers) {
      const teacherId = teacher.teacher_id;
      teacherUsedHours[teacherId] = 0;

      let placed = 0;

      for (const slotInfo of unassignedSlots) {
        if (teacherUsedHours[teacherId] >= teacher.max_hours) break;

        if (canPlaceTeacher(teacherId, slotInfo.class_id, slotInfo.day, slotInfo.slot)) {
          if (assignTeacherToSlot(teacherId, slotInfo.class_id, slotInfo.day, slotInfo.slot)) {
            placed++;
          }
        }
      }

      if (placed < teacher.max_hours) {
        console.warn(`Teacher ${teacher.name} could only be placed for ${placed}/${teacher.max_hours} hours`);
      }
    }
  }

  function autoPlaceTeachers() {
    try {
      teacherUsedHours = {};

      clearAllSlots();

      setTimeout(() => {
        placeClassSpecificTeachers();

        setTimeout(() => {
          placeDistributedTeachers();

          setTimeout(() => {
            if (typeof updateHours === 'function') {
              updateHours();
            }

            if (typeof trackChanges === 'function') {
              trackChanges();
            }

            showMsg('Auto-placement complete! Remember to save your changes.', true, 'status2');
          }, 100);
        }, 100);
      }, 100);

    } catch (error) {
      console.error('Error during auto-placement:', error);
      showMsg('Error during auto-placement: ' + error.message, false, 'status2');
    }
  }

  function showMsg(msg, ok, target) {
    if (typeof window.showMsg === 'function') {
      window.showMsg(msg, ok, target);
    } else {
      const el = document.getElementById(target);
      if (el) {
        el.textContent = msg;
        el.className = ok ? 'status-success' : 'status-error';
        el.style.display = 'inline-block';
        setTimeout(() => {
          el.textContent = '';
          el.className = '';
          el.style.display = '';
        }, 5000);
      } else {
        alert(msg);
      }
    }
  }

  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', createAutoPlaceButton);
  } else {
    createAutoPlaceButton();
  }

})();

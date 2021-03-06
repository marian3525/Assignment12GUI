#pragma once
#include "GUI.h"
using namespace std;
GUI::GUI(QWidget *parent) : QMainWindow(parent)
{	
	ui.setupUi(this);
	buildWindow();
	tutorialModel = new QStringListModel(this);
	watchlistModel = new QStringListModel(this);  //?
	bindWidgets();
	connect();
	configureGroups();
	adminRadio->click();				//set the initial state to admin	
	tutorialList->setSelectionMode(QAbstractItemView::ExtendedSelection);
	watchlist->setSelectionMode(QAbstractItemView::ExtendedSelection);
	updateList(tutorialList, tutorialModel, tutorialStrList, "main");	//populate the main list from the main repo
	//Default writer if nothing is selected
	this->controller.setMode((Writer*) new HTMLWriter{ "watchlist.html" });
}

void GUI::buildWindow() {

	statusBar()->showMessage("App started");
}

void GUI::bindWidgets() {
	/*
		Bind the widgets in the UI to the references
	*/
	//menu:
	actionCSV = findChild<QAction*>("actionCSV");
	actionHTML = findChild<QAction*>("actionHTML");
	//admin operations
	addButton = findChild<QPushButton*>("addTutorialButton");
	removeButton = findChild<QPushButton*>("removeTutorialButton");
	updateButton = findChild<QPushButton*>("updateTutorialButton");
	//user operations
	likeButton = findChild<QPushButton*>("likeButton");
	addToWatchlistButton = findChild<QPushButton*>("addToWatchlistButton");
	removeFromWatchlistButton = findChild<QPushButton*>("removeFromWatchlistButton");
	watchButton = findChild<QPushButton*>("watchButton");
	filterButton = findChild<QPushButton*>("filterButton");
	backButton = findChild<QPushButton*>("backButton");
	//input fields
	titleInput = findChild<QPlainTextEdit*>("titleInput");
	presenterInput = findChild<QPlainTextEdit*>("presenterInput");
	durationInput = findChild<QPlainTextEdit*>("durationInput");
	likesInput = findChild<QPlainTextEdit*>("likesInput");
	linkInput = findChild<QPlainTextEdit*>("linkInput");
	filterInput = findChild<QPlainTextEdit*>("filterInput");
	//lists
	tutorialList = findChild<QListView*>("tutorialList");
	tutorialList->setEditTriggers(QAbstractItemView::NoEditTriggers);  //read only
	watchlist = findChild<QListView*>("watchlist");
	watchlist->setEditTriggers(QAbstractItemView::NoEditTriggers);		//read only

	//mode select
	adminRadio = findChild<QRadioButton*>("administratorRadio");
	userRadio = findChild<QRadioButton*>("userRadio");
	//exit
	exitButton = findChild<QPushButton*>("exitButton");
	//combo box
	displayMode = findChild<QComboBox*>("comboBox");
	//shortcuts
	undoShortcut = new QShortcut(this);
	undoShortcut->setKey(Qt::CTRL + Qt::Key_Z);
	undoButton = findChild<QPushButton*>("undoButton");

	redoShortcut = new QShortcut(this);
	redoShortcut->setKey(Qt::CTRL + Qt::Key_Y);
	redoButton = findChild<QPushButton*>("redoButton");

	//show list window button
	showListButton = findChild<QPushButton*>("showListButton");
}

void GUI::connect() {
	/**
		Connect the Slots to corresponding signals
	**/
	QObject::connect(actionCSV, &QAction::changed, this, &GUI::onCSVSelected);
	QObject::connect(actionHTML, &QAction::changed, this, &GUI::onHTMLSelected);

	QObject::connect(addButton, &QPushButton::clicked, this, &GUI::onAdd);
	QObject::connect(removeButton, &QPushButton::clicked, this, &GUI::onRemove);
	QObject::connect(updateButton, &QPushButton::clicked, this, &GUI::onUpdate);

	QObject::connect(likeButton, &QPushButton::clicked, this, &GUI::onLike);
	QObject::connect(addToWatchlistButton, &QPushButton::clicked, this, &GUI::onUserAdd);
	QObject::connect(removeFromWatchlistButton, &QPushButton::clicked, this, &GUI::onUserRemove);
	QObject::connect(watchButton, &QPushButton::clicked, this, &GUI::onWatch);

	QObject::connect(exitButton, &QPushButton::clicked, this, &QApplication::exit);

	QObject::connect(adminRadio, &QPushButton::clicked, this, &GUI::onAdminRadioCheck);
	QObject::connect(userRadio, &QPushButton::clicked, this, &GUI::onUserRadioCheck);
	QObject::connect(filterButton, &QPushButton::clicked, this, &GUI::onFilter);
	QObject::connect(backButton, &QPushButton::clicked, this, &GUI::onBack);

	QObject::connect(displayMode, &QComboBox::currentTextChanged, this, &GUI::onDisplayChange);

	QObject::connect(undoShortcut, &QShortcut::activated, this, &GUI::onUndo);
	QObject::connect(undoButton, &QPushButton::clicked, this, &GUI::onUndo);

	QObject::connect(redoShortcut, &QShortcut::activated, this, &GUI::onRedo);
	QObject::connect(redoButton, &QPushButton::clicked, this, &GUI::onRedo);
	QObject::connect(showListButton, &QPushButton::clicked, this, &GUI::onShowListWindow);
}

void GUI::onClick() {
	popWarning("title", "content");
}

void GUI::onExit() {

}

void GUI::onBack()
{
	/*
		Restore the main list with the elems from the main repo
	*/
	updateList(tutorialList, tutorialModel, tutorialStrList, "main");
}

void GUI::popWarning(string title, string message) {
	QMessageBox::warning(0, title.c_str(), message.c_str());
}

void GUI::updateStatus(string status) {
	statusBar()->showMessage("App started");
}

void GUI::onAdd() {
	/**
		Read the data from the title, presenter, duration, likes and link field
		Validate them, pop an error if invalid or add them to the repo if valid
	*/
	string title="", presenter="", duration="", likes="", link="";
	int durationSeconds = 0, durationMinutes = 0;
	int likesInt = 0;

	//read data from the interface
	title = titleInput->toPlainText().toStdString();
	presenter = presenterInput->toPlainText().toStdString();
	duration = durationInput->toPlainText().toStdString();
	likes = likesInput->toPlainText().toStdString();
	link = linkInput->toPlainText().toStdString();

	try {
		durationSeconds = getSeconds(duration);
		durationMinutes = getMinutes(duration);
		likesInt = stoi(likes);

		Validator::validateAdd(title, presenter, durationSeconds + durationMinutes*60, likesInt, link);
	}
	catch (ValidatorException& ve) {
		//validator failed
		QMessageBox::warning(0, "Invalid input", ve.getMessage().c_str());
		return;

	}
	catch (const std::invalid_argument iarg) {
		//invalid likes
		QMessageBox::warning(0, "Invalid input", "Likes must be a positive integer");
		return;
	}
	//passed validation, attempt to add and update the views
	try {
		controller.addTutorial(title, presenter, durationSeconds + durationMinutes * 60, likesInt, link);
		updateList(tutorialList, tutorialModel, tutorialStrList, "main");
	}
	catch (ControllerException& ce) {
		QMessageBox::warning(0, "Couldn't add element", "The element with the given title already exists!");
		return;
	}
}

void GUI::onRemove() {
	/**
		Read data from the title field, validate and remove if valid.
	*/
	string title = titleInput->toPlainText().toStdString();
	try {
		controller.removeTutorial(title);
		updateList(tutorialList, tutorialModel, tutorialStrList, "main");
	}
	catch (ValidatorException& ve) {
		QMessageBox::warning(0, "Invalid input", ve.getMessage().c_str());
	}
	catch (ControllerException& ce) {
		QMessageBox::warning(0, "Couldn't remove item", ce.getMessage().c_str());
	}
}

void GUI::onUpdate() {
	/*
		Read the data from the title, presenter, duration, likes and link field.
		Update the entry with the given title if all the params are valid
	*/
	string title = "", presenter = "", duration = "", likes = "", link = "";
	int durationSeconds = 0, durationMinutes = 0;
	int likesInt = 0;

	//read data from the interface
	title = titleInput->toPlainText().toStdString();
	presenter = presenterInput->toPlainText().toStdString();
	duration = durationInput->toPlainText().toStdString();
	likes = likesInput->toPlainText().toStdString();
	link = linkInput->toPlainText().toStdString();

	try {
		durationSeconds = getSeconds(duration);
		durationMinutes = getMinutes(duration);
		likesInt = stoi(likes);

		Validator::validateAdd(title, presenter, durationSeconds + durationMinutes * 60, likesInt, link);
	}
	catch (ValidatorException& ve) {
		//validator failed
		QMessageBox::warning(0, "Invalid input", ve.getMessage().c_str());
		return;

	}
	catch (const std::invalid_argument iarg) {
		//invalid likes
		QMessageBox::warning(0, "Invalid input", "Likes must be a positive integer");
		return;
	}
	//passed validation, attempt to add and update the views
	try {
		controller.updateTutorial(title, presenter, durationSeconds + durationMinutes * 60, likesInt, link);
		updateList(tutorialList, tutorialModel, tutorialStrList, "main");
	}
	catch (ControllerException& ce) {
		QMessageBox::warning(0, "Couldn't update element", ce.getMessage().c_str());
		return;
	}
}

void GUI::onAdminRadioCheck() {
	qDebug() << "Admin mode";
	if (mode == "user") {
		//the mode toggled
		mode = "admin";
		onModeChange();
	}
	//same mode
}

void GUI::onUserRadioCheck() {
	qDebug() << "User mode";
	if (mode == "admin") {
		//mode toggled
		mode = "user";
		onModeChange();
	}
	mode = "user";
}


void GUI::onWatch() {
	/*
		Called when the user clicks the watch button. Create the output file in the required format
		and open an app to view it
	*/
	controller.write();
}

void GUI::onFilter() {
	/*
		Called when clicking the filter button IN USER MODE. Populate the list with the results
	*/
	string presenter = filterInput->toPlainText().toStdString();
	vector<Tutorial> results = controller.filterByPresenter(presenter);
	vector<string> strings;
	for_each(results.begin(), results.end(), [&strings](const Tutorial& tutorial) { strings.push_back(tutorial.toString()); });
	//update main list using the strings

	if (strings.size() == 0) {
		tutorialModel->setStringList(tutorialStrList);
		tutorialList->setModel(tutorialModel);
		return;
	}

	tutorialStrList.clear();		//the filtered list gets appended for each new flitering unless cleared

	for_each(strings.begin(), strings.end(), [&](const string& element) {
		tutorialStrList.append(QString::fromStdString(element));
		tutorialModel->setStringList(tutorialStrList);
		tutorialList->setModel(tutorialModel);
	});
}

string GUI::getTitleFromString(string elem) {
	/*
	Extract the title of a tutorial from its string representation
	@param: elem: the tutorial string
	@return: string representing the title of the tutorial with the string repr. <elem>
	*/
	char title[50];
	int start = elem.find(' ', 9);  //the first space is at pos. 8, ignore it
	start += 1;
	int end = elem.find('\n');
	end -= 1;
	//the title at [start, end]
	elem.copy(title, end - start + 1, start);
	//terminator!
	title[end - start + 1] = 0;
	return string(title);
}


string GUI::getTitleFromShortString(string elem) {
	char title[50];
	int start = 0;
	int end = elem.find(' ');
	end -= 1;
	elem.copy(title, end - start + 1, start);
	title[end - start + 1] = 0;
	return string(title);
}

void GUI::onLike() {
	/*
		Called when clicking the like button. Update one or more tutorials in both repos
	*/
	QModelIndexList indexes;
	indexes = tutorialList->selectionModel()->selectedIndexes();
	//for_each(indexes.begin(), indexes.end(), [](const QModelIndex& index) {qDebug() << index; });
	
	for (const QModelIndex& index:indexes) {
		string title;
		QString elem = tutorialList->model()->data(index).toString();
		if (displayCode == 0)
			title = getTitleFromString(elem.toStdString());
		else
			title = getTitleFromShortString(elem.toStdString());
		controller.likeTutorial(title);
	}
	//updateList(tutorialList, tutorialModel, tutorialStrList, "main");//update from the main repo, where the likes were modified
	//updateList(watchlist, watchlistModel, watchlistStrList, "watchlist");
	update();
}

void GUI::onUserAdd() {
	/*
		Add the selected items from the main list to the watchlist
	*/
	QModelIndexList indexes;
	indexes = tutorialList->selectionModel()->selectedIndexes();
	try {
		for (const QModelIndex& index : indexes) {
			string title;
			QString elem = tutorialList->model()->data(index).toString();
			if (displayCode == 0)
				title = getTitleFromString(elem.toStdString());
			else
				title = getTitleFromShortString(elem.toStdString());
			controller.addToWatchList(title);
		}
	}
	catch (ControllerException& ce) {
		QMessageBox::warning(0, "Invalid operation", "Item alreay in the watchlist");
	}

	updateList(watchlist, watchlistModel, watchlistStrList, "watchlist");
}

void GUI::onUserRemove() {
	/*
		Called when a user clicks the remove button with a tutorial from their watchlist selected
	*/
	QModelIndexList indexes;
	indexes = watchlist->selectionModel()->selectedIndexes();
	try {
		for (const QModelIndex& index : indexes) {
			string title;
			QString elem = watchlist->model()->data(index).toString();
			if (displayCode == 0)
				title = getTitleFromString(elem.toStdString());
			else
				title = getTitleFromShortString(elem.toStdString());
			controller.deleteFromWatchlist(title);
		}
	}
	catch (RepositoryException& re) {
		QMessageBox::warning(0, "Invalid operation", "Can't remove nonexistent element");
	}
	updateList(watchlist, watchlistModel, watchlistStrList, "watchlist");
}

void GUI::onHTMLSelected() {
	if (actionCSV->isChecked())
		actionCSV->setChecked(false);
	else
		actionHTML->setChecked(true);

	controller.setMode((Writer*) new HTMLWriter{ "watchlist.html" });
	qDebug() << "HTMLSelecred";

}

void GUI::onCSVSelected() {
	if (actionHTML->isChecked())
		actionHTML->setChecked(false);
	else
		actionCSV->setChecked(true);

	controller.setMode((Writer*) new CSVWriter{ "watchlist.csv" });
	
	qDebug() << "CSVSelecred";

}

void GUI::onModeChange() {
	/*
		Called when the mode changes.
		Activate/deactivate the required widgets
		When admin: deactivate all except the input fields, the radio check and the 3 admin mode buttons
		When user: deactivate the ones above and activate the others
	*/
	qDebug() << "onModeChange";
	if (mode == "admin") {
		for_each(adminGroup.begin(), adminGroup.end(), [](QWidget* widget) {widget->setDisabled(false); });
		for_each(userGroup.begin(), userGroup.end(), [](QWidget* widget) {widget->setDisabled(true); });
	}
	else {
		for_each(adminGroup.begin(), adminGroup.end(), [](QWidget* widget) {widget->setDisabled(true); });
		for_each(userGroup.begin(), userGroup.end(), [](QWidget* widget) {widget->setDisabled(false); });
	}
}


void GUI::onDisplayChange() {
	displayCode = displayMode->currentIndex();
	update();
}

void GUI::onUndo() {
	controller.undo();
	//update the list
	update();
}

void GUI::onRedo() {
	controller.redo();
	//update the list
	update();

}

void GUI::onShowListWindow() {
	//QStringList strlist;
	//listWindow.update(strlist);
	update();
	listWindow.show();
	
}

void GUI::updateList(QListView* widget, QStringListModel* model, QStringList strList, string repoMode) {
	/*
		@param: widget: the view to update
		@param: repoMode: "main" or "watchlist", selects from which repo the data will be drawn
	*/
	vector<string> all;

	if (repoMode == "main") {
		if (displayCode == 0)		//detailed
			all = controller.getAllPrintable();
		else
			all = controller.getAllPrintableShort();
	}
	if (repoMode == "watchlist"){
		if (displayCode == 0) {
			all = controller.getWatchlistPrintable();
		}
		else {
			all = controller.getAllPrintableShortWatchlist();
		}
	}
	strList.clear();

	if (all.size() == 0) {
		model->setStringList(strList);
		widget->setModel(model);
		return;
	}
	for_each(all.begin(), all.end(), [&](const string& element) { 	
		strList.append(QString::fromStdString(element));
		model->setStringList(strList);
		widget->setModel(model); 
	});
}

void GUI::appendToList(QListView* list, QStringListModel* model, QStringList strList, string element) {
	
	strList.append(QString::fromStdString(element));
	model->setStringList(strList);
	list->setModel(model);
}

#pragma warning(disable : 4996)
int GUI::getMinutes(string s) {
	/**
	* s: string in the format xmys, where x and y are integers
	* Return the number of minutes as an int
	*/
	if (s == "") {
		return -1;
	}
	int end = s.find('m');
	char aux[10];
	s.copy(aux, end);
	return stoi(aux);
}

#pragma warning(disable : 4996)
int GUI::getSeconds(string s) {
	/**
	* s: string in the format xmys, where x and y are integers
	* Return the number of seconds as an int, -1 if the string is empty
	*/
	char aux[10]; //needed for s.copy() later

	//10m14s
	if (s == "") {
		return -1;
	}
	int start = s.find('m');
	if (start == string::npos) {
		return -1;
	}
	start += 1;
	int end = s.find('s', start);
	if (end == string::npos) {
		return -1;
	}
	int len = end - start;
	s.copy(aux, len, start);
	aux[start + len] = 0;
	return stoi(aux);
}

void GUI::configureGroups() {
	adminGroup.push_back(titleInput);
	adminGroup.push_back(presenterInput);
	adminGroup.push_back(durationInput);
	adminGroup.push_back(likesInput);
	adminGroup.push_back(linkInput);

	adminGroup.push_back(addButton);
	adminGroup.push_back(removeButton);
	adminGroup.push_back(updateButton);

	userGroup.push_back(likeButton);
	userGroup.push_back(addToWatchlistButton);
	userGroup.push_back(removeFromWatchlistButton);
	userGroup.push_back(watchButton);
	userGroup.push_back(filterButton);
	userGroup.push_back(backButton);

	userGroup.push_back(filterInput);
}

void GUI::update() {
	updateList(tutorialList, tutorialModel, tutorialStrList, "main");
	updateList(watchlist, watchlistModel, watchlistStrList, "watchlist");

	QStringList strlist;
	vector<string> all = controller.getWatchlistPrintable();
	for_each(all.begin(), all.end(), [&strlist](string elem) {
		strlist.append(QString::fromStdString(elem));
	});
	listWindow.update(strlist);
}

int GUI::getSize()
{
	return controller.getWatchList().size();
}

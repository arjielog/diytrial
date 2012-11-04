/* hello.c */

/* A microhttpd application to be run under the Red Hat OpenShift PaaS
   DIY cartridge.  Demonstrates running a binary application in this
   PaaS */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <microhttpd.h>

static int
answer_to_connection(void *cls,
		     struct MHD_Connection *connection,
		     const char *url,
		     const char *method,
		     const char *version,
		     const char *upload_data, size_t *upload_data_size,
		     void **con_cls)
{
  const char *page = "<html><body>




odle frontpage.
 *
 * @package    core
 * @copyright  1999 onwards Martin Dougiamas (http://dougiamas.com)
 * @license    http://www.gnu.org/copyleft/gpl.html GNU GPL v3 or later
 */

    if (!file_exists('./config.php')) {
        header('Location: install.php');
        die;
    }

    require_once('config.php');
    require_once($CFG->dirroot .'/course/lib.php');
    require_once($CFG->libdir .'/filelib.php');

    redirect_if_major_upgrade_required();

    $urlparams = array();
    if (!empty($CFG->defaulthomepage) && ($CFG->defaulthomepage == HOMEPAGE_MY) && optional_param('redirect', 1, PARAM_BOOL) === 0) {
        $urlparams['redirect'] = 0;
    }
    $PAGE->set_url('/', $urlparams);
    $PAGE->set_course($SITE);

    // Prevent caching of this page to stop confusion when changing page after making AJAX changes
    $PAGE->set_cacheable(false);

    if ($CFG->forcelogin) {
        require_login();
    } else {
        user_accesstime_log();
    }

    $hassiteconfig = has_capability('moodle/site:config', get_context_instance(CONTEXT_SYSTEM));

/// If the site is currently under maintenance, then print a message
    if (!empty($CFG->maintenance_enabled) and !$hassiteconfig) {
        print_maintenance_message();
    }

    if ($hassiteconfig && moodle_needs_upgrading()) {
        redirect($CFG->wwwroot .'/'. $CFG->admin .'/index.php');
    }

    if (get_home_page() != HOMEPAGE_SITE) {
        // Redirect logged-in users to My Moodle overview if required
        if (optional_param('setdefaulthome', false, PARAM_BOOL)) {
            set_user_preference('user_home_page_preference', HOMEPAGE_SITE);
        } else if (!empty($CFG->defaulthomepage) && ($CFG->defaulthomepage == HOMEPAGE_MY) && optional_param('redirect', 1, PARAM_BOOL) === 1) {
            redirect($CFG->wwwroot .'/my/');
        } else if (!empty($CFG->defaulthomepage) && ($CFG->defaulthomepage == HOMEPAGE_USER)) {
            $PAGE->settingsnav->get('usercurrentsettings')->add(get_string('makethismyhome'), new moodle_url('/', array('setdefaulthome'=>true)), navigation_node::TYPE_SETTING);
        }
    }

    if (isloggedin()) {
        add_to_log(SITEID, 'course', 'view', 'view.php?id='.SITEID, SITEID);
    }

/// If the hub plugin is installed then we let it take over the homepage here
    if (file_exists($CFG->dirroot.'/local/hub/lib.php') and get_config('local_hub', 'hubenabled')) {
        require_once($CFG->dirroot.'/local/hub/lib.php');
        $hub = new local_hub();
        $continue = $hub->display_homepage();
        //display_homepage() return true if the hub home page is not displayed
        //mostly when search form is not displayed for not logged users
        if (empty($continue)) {
            exit;
        }
    }

    $PAGE->set_pagetype('site-index');
    $PAGE->set_other_editing_capability('moodle/course:manageactivities');
    $PAGE->set_docs_path('');
    $PAGE->set_pagelayout('frontpage');
    $editing = $PAGE->user_is_editing();
    $PAGE->set_title($SITE->fullname);
    $PAGE->set_heading($SITE->fullname);
    echo $OUTPUT->header();

/// Print Section or custom info
    get_all_mods($SITE->id, $mods, $modnames, $modnamesplural, $modnamesused);
    if (!empty($CFG->customfrontpageinclude)) {
        include($CFG->customfrontpageinclude);

    } else if ($SITE->numsections > 0) {

        if (!$section = $DB->get_record('course_sections', array('course'=>$SITE->id, 'section'=>1))) {
            $DB->delete_records('course_sections', array('course'=>$SITE->id, 'section'=>1)); // Just in case
            $section = new stdClass();
            $section->course = $SITE->id;
            $section->section = 1;
            $section->summary = '';
            $section->summaryformat = FORMAT_HTML;
            $section->sequence = '';
            $section->visible = 1;
            $section->id = $DB->insert_record('course_sections', $section);
            rebuild_course_cache($SITE->id, true);
        }

        if (!empty($section->sequence) or !empty($section->summary) or $editing) {
            echo $OUTPUT->box_start('generalbox sitetopic');

            /// If currently moving a file then show the current clipboard
            if (ismoving($SITE->id)) {
                $stractivityclipboard = strip_tags(get_string('activityclipboard', '', $USER->activitycopyname));
                echo '<p><font size="2">';
                echo "$stractivityclipboard&nbsp;&nbsp;(<a href=\"course/mod.php?cancelcopy=true&amp;sesskey=".sesskey()."\">". get_string('cancel') .'</a>)';
                echo '</font></p>';
            }

            $context = get_context_instance(CONTEXT_COURSE, SITEID);
            $summarytext = file_rewrite_pluginfile_urls($section->summary, 'pluginfile.php', $context->id, 'course', 'section', $section->id);
            $summaryformatoptions = new stdClass();
            $summaryformatoptions->noclean = true;
            $summaryformatoptions->overflowdiv = true;

            echo format_text($summarytext, $section->summaryformat, $summaryformatoptions);

            if ($editing) {
                $streditsummary = get_string('editsummary');
                echo "<a title=\"$streditsummary\" ".
                     " href=\"course/editsection.php?id=$section->id\"><img src=\"" . $OUTPUT->pix_url('t/edit') . "\" ".
                     " class=\"iconsmall\" alt=\"$streditsummary\" /></a><br /><br />";
            }

            print_section($SITE, $section, $mods, $modnamesused, true);

            if ($editing) {
                print_section_add_menus($SITE, $section->section, $modnames);
            }
            echo $OUTPUT->box_end();
        }
    }
    // Include course AJAX
    if (include_course_ajax($SITE, $modnamesused)) {
        // Add the module chooser
        $renderer = $PAGE->get_renderer('core', 'course');
        echo $renderer->course_modchooser(get_module_metadata($SITE, $modnames), $SITE);
    }

    if (isloggedin() and !isguestuser() and isset($CFG->frontpageloggedin)) {
        $frontpagelayout = $CFG->frontpageloggedin;
    } else {
        $frontpagelayout = $CFG->frontpage;
    }

    foreach (explode(',',$frontpagelayout) as $v) {
        switch ($v) {     /// Display the main part of the front page.
            case FRONTPAGENEWS:
                if ($SITE->newsitems) { // Print forums only when needed
                    require_once($CFG->dirroot .'/mod/forum/lib.php');

                    if (! $newsforum = forum_get_course_forum($SITE->id, 'news')) {
                        print_error('cannotfindorcreateforum', 'forum');
                    }

                    // fetch news forum context for proper filtering to happen
                    $newsforumcm = get_coursemodule_from_instance('forum', $newsforum->id, $SITE->id, false, MUST_EXIST);
                    $newsforumcontext = get_context_instance(CONTEXT_MODULE, $newsforumcm->id, MUST_EXIST);

                    $forumname = format_string($newsforum->name, true, array('context' => $newsforumcontext));
                    echo html_writer::tag('a', get_string('skipa', 'access', textlib::strtolower(strip_tags($forumname))), array('href'=>'#skipsitenews', 'class'=>'skip-block'));

                    if (isloggedin()) {
                        $SESSION->fromdiscussion = $CFG->wwwroot;
                        $subtext = '';
                        if (forum_is_subscribed($USER->id, $newsforum)) {
                            if (!forum_is_forcesubscribed($newsforum)) {
                                $subtext = get_string('unsubscribe', 'forum');
                            }
                        } else {
                            $subtext = get_string('subscribe', 'forum');
                        }
                        echo $OUTPUT->heading($forumname, 2, 'headingblock header');
                        $suburl = new moodle_url('/mod/forum/subscribe.php', array('id' => $newsforum->id, 'sesskey' => sesskey()));
                        echo html_writer::tag('div', html_writer::link($suburl, $subtext), array('class' => 'subscribelink'));
                    } else {
                        echo $OUTPUT->heading($forumname, 2, 'headingblock header');
                    }

                    forum_print_latest_discussions($SITE, $newsforum, $SITE->newsitems, 'plain', 'p.modified DESC');
                    echo html_writer::tag('span', '', array('class'=>'skip-block-to', 'id'=>'skipsitenews'));
                }
            break;

            case FRONTPAGECOURSELIST:
                if (isloggedin() and !$hassiteconfig and !isguestuser() and empty($CFG->disablemycourses)) {
                    echo html_writer::tag('a', get_string('skipa', 'access', textlib::strtolower(get_string('mycourses'))), array('href'=>'#skipmycourses', 'class'=>'skip-block'));
                    echo $OUTPUT->heading(get_string('mycourses'), 2, 'headingblock header');
                    print_my_moodle();
                    echo html_writer::tag('span', '', array('class'=>'skip-block-to', 'id'=>'skipmycourses'));
                } else if ((!$hassiteconfig and !isguestuser()) or ($DB->count_records('course') <= FRONTPAGECOURSELIMIT)) {
                    // admin should not see list of courses when there are too many of them
                    echo html_writer::tag('a', get_string('skipa', 'access', textlib::strtolower(get_string('availablecourses'))), array('href'=>'#skipavailablecourses', 'class'=>'skip-block'));
                    echo $OUTPUT->heading(get_string('availablecourses'), 2, 'headingblock header');
                    print_courses(0);
                    echo html_writer::tag('span', '', array('class'=>'skip-block-to', 'id'=>'skipavailablecourses'));
                }
            break;

            case FRONTPAGECATEGORYNAMES:
                echo html_writer::tag('a', get_string('skipa', 'access', textlib::strtolower(get_string('categories'))), array('href'=>'#skipcategories', 'class'=>'skip-block'));
                echo $OUTPUT->heading(get_string('categories'), 2, 'headingblock header');
                echo $OUTPUT->box_start('generalbox categorybox');
                print_whole_category_list(NULL, NULL, NULL, -1, false);
                echo $OUTPUT->box_end();
                print_course_search('', false, 'short');
                echo html_writer::tag('span', '', array('class'=>'skip-block-to', 'id'=>'skipcategories'));
            break;

            case FRONTPAGECATEGORYCOMBO:
                echo html_writer::tag('a', get_string('skipa', 'access', textlib::strtolower(get_string('courses'))), array('href'=>'#skipcourses', 'class'=>'skip-block'));
                echo $OUTPUT->heading(get_string('courses'), 2, 'headingblock header');
                $renderer = $PAGE->get_renderer('core','course');
                // if there are too many courses, budiling course category tree could be slow,
                // users should go to course index page to see the whole list.
                $coursecount = $DB->count_records('course');
                if (empty($CFG->numcoursesincombo)) {
                    // if $CFG->numcoursesincombo hasn't been set, use default value 500
                    $CFG->numcoursesincombo = 500;
                }
                if ($coursecount > $CFG->numcoursesincombo) {
                    $link = new moodle_url('/course/');
                    echo $OUTPUT->notification(get_string('maxnumcoursesincombo', 'moodle', array('link'=>$link->out(), 'maxnumofcourses'=>$CFG->numcoursesincombo, 'numberofcourses'=>$coursecount)));
                } else {
                    echo $renderer->course_category_tree(get_course_category_tree());
                }
                print_course_search('', false, 'short');
                echo html_writer::tag('span', '', array('class'=>'skip-block-to', 'id'=>'skipcourses'));
            break;

            case FRONTPAGETOPICONLY:    // Do nothing!!  :-)
            break;

        }
        echo '<br />';
    }




    echo $OUTPUT->footer();
</body></html>";
  struct MHD_Response *response;
  int ret;

  fprintf(stderr, "respond to %s %s\n", method, url);

  response =
    MHD_create_response_from_buffer(strlen(page), (void *)page, 
				    MHD_RESPMEM_PERSISTENT);
  ret =
    MHD_queue_response(connection, MHD_HTTP_OK, response);

  MHD_destroy_response(response);

  return ret;
}

int main (int argc, char *argv[])
{
  struct MHD_Daemon *daemon;
  struct sockaddr_in sad;
  unsigned short port;
  int ret;
  char *env_openshift_ipstr;
  char *env_openshift_port;

  env_openshift_ipstr = getenv("OPENSHIFT_INTERNAL_IP");
  env_openshift_port = getenv("OPENSHIFT_INTERNAL_PORT");

  if (env_openshift_ipstr == NULL) {
    fprintf(stderr, "OPENSHIFT_INTERNAL_IP is not defined\n");
    return 1;
  }
  if (env_openshift_port == NULL) {
    fprintf(stderr, "OPENSHIFT_INTERNAL_PORT is not defined\n");
    return 1;
  }

  fprintf(stderr, "OPENSHIFT_INTERNAL_IP is %s\n",  env_openshift_ipstr);
  fprintf(stderr, "OPENSHIFT_INTERNAL_PORT is %s\n", env_openshift_port);

  port = (unsigned short) atoi(env_openshift_port);
  if (port == 0) {
    fprintf(stderr, "could not parse OPENSHIFT_INTERNAL_PORT\n");
    return 1;
  }

  sad.sin_family = AF_INET;
  sad.sin_port = htons(port);
  ret = inet_pton(AF_INET,
		  env_openshift_ipstr,
		  &(sad.sin_addr.s_addr));
  if (ret != 1) {
    fprintf(stderr, "could not parse OPENSHIFT_INTERNAL_ADDRESS\n");
    return 1;
  }

  daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY
			    | MHD_USE_DEBUG,
			    htons(port),
			    NULL, NULL,
			    &answer_to_connection, NULL,
			    MHD_OPTION_SOCK_ADDR, (struct sockaddr *) &(sad),
			    MHD_OPTION_END);
  if (NULL == daemon) {
    fprintf(stderr, "fail MHD_start_daemon\n");
    return 1;
  }

  while (1) {  /* run forever, until killed */
    fprintf(stderr, "loop\n");
    sleep(60);
  }

  MHD_stop_daemon(daemon);
  return 0;
}
